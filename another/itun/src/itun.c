
#include <sys/socket.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include <getopt.h>

#include "buffer.h"
#include "common.h"

#define DEFAULT_PORT		"1771"

struct init_params *params = NULL;

struct connection_data
{
	struct itun_buffer *proxy_buffer;
	struct itun_buffer *client_buffer;

	int connfd;
	int connid;

	int receive_seq;
	int send_seq;

	pthread_t proxy_write;
	pthread_t client_read;
	pthread_t client_write;
};

static void send_packet(struct connection_data *data, int type, void *payload, int size)
{
	struct itun_header *packet = malloc(sizeof(struct itun_header) + size);
	bzero(packet, sizeof(struct itun_header) + size);

	if (payload != NULL)
		memcpy((char *) packet + sizeof(struct itun_header), payload, size);

	packet->magic		= MAGIC_NUMBER;
	packet->id		= data->connid;
	packet->type		= type;
	packet->seq		= data->send_seq++;
	packet->length		= size;

	send_icmp_packet(params->src_ip, params->dst_ip, packet);

	free(packet);
}

static struct connection_data* init_data(int connfd)
{
	struct connection_data *data = malloc(sizeof(struct connection_data));
	bzero(data, sizeof(struct connection_data));

	data->proxy_buffer	= buffer_new();
	data->client_buffer	= buffer_new();

	data->connfd		= connfd;
	data->connid		= params->last_id++;

	return data;
}

static void free_data(struct connection_data *data)
{
	if (data->connfd != 0)
		close(data->connfd);
	if (data->proxy_write != 0)
		pthread_cancel(data->proxy_write);
	if (data->client_read != 0)
		pthread_cancel(data->client_read);
	if (data->client_write != 0)
		pthread_cancel(data->client_write);
	if (data->client_buffer != NULL)
		buffer_free(data->client_buffer);
	if (data->proxy_buffer != NULL)
		buffer_free(data->proxy_buffer);
	free(data);
}

static void* do_client_write(void *arg)
{
	struct connection_data *data = (struct connection_data *) arg;

	while (1)
	{
		struct buffer_chunk *chunk = buffer_take(data->proxy_buffer);

		int done = 0;
		while (1)
		{
			int writed = write(data->connfd, chunk->data + done, chunk->size - done);
			if (writed == -1)
			{
				if (errno != EINTR)
					break;
				continue;
			}

			if (writed == 0)
				break;

			done += writed;
			printf("L->C writed %d bytes to connection %d\n", writed, data->connid);
		}

		buffer_free_chunk(chunk);
	}

	pthread_exit(NULL);
}

static void* do_client_read(void *arg)
{
	struct connection_data *data = (struct connection_data *) arg;

	char *temp_buf = malloc((SOCKET_BUFFER_SIZE + 1) * sizeof(char));

	while (1)
	{
		int readed = read(data->connfd, temp_buf, SOCKET_BUFFER_SIZE);
		if (readed == -1)
		{
			if (errno != EINTR)
				break;
			continue;
		}

		if (readed == 0)
			break;

		temp_buf[readed] = 0;

		printf("C->L readed %d bytes for connection %d\n", readed, data->connid);
		printf("%s\n", temp_buf);

		buffer_add(data->client_buffer, temp_buf, readed);
	}

	free(temp_buf);
	close(data->connfd);

	printf("Connection with client %d closed\n", data->connid);
	send_packet(data, TYPE_CONNECTION_CLOSED, NULL, 0);

	pthread_exit(NULL);
}

static void* do_proxy_write(void *arg)
{
	struct connection_data *data = (struct connection_data *) arg;

	while (1)
	{
		struct buffer_chunk *chunk = buffer_take(data->client_buffer);

		printf("L->P writed %d bytes to connection %d\n", chunk->size, data->connid);

		send_packet(data, TYPE_CLIENT_DATA, chunk->data, chunk->size);
		buffer_free_chunk(chunk);
	}

	pthread_exit(NULL);
}

static void* do_proxy_read(void *arg)
{
	if (arg) {}

	struct pcap_pkthdr *header;
	const u_char *packet;
	int result;

	while ((result = pcap_next_ex(params->libpcap, &header, &packet)) >= 0)
	{
		if (result == 0)
			continue;

		struct itun_packet *itp = parse_packet(header->len, packet);
		if (itp == NULL)
			continue;

		if (itp->icmp_type == ICMP_ECHOREPLY)
		{
			if ((itp->itun->type & PROXY_FLAG) == PROXY_FLAG)
				continue;

			printf("Received reply to packet %d for connection %d\n", itp->itun->seq, itp->itun->id);
			continue;
		}

		if ((itp->itun->type & PROXY_FLAG) != PROXY_FLAG)
			continue;

		struct connection_data *data = (struct connection_data *) ring_get(params->connections, itp->itun->id);
		if (data == NULL)
		{
			free(itp);
			continue;
		}

		switch (itp->itun->type)
		{
			case TYPE_CONNECT_FAILED:
			{
				printf("Proxy connection %d failed to connect with server\n", data->connid);

				ring_remove(params->connections, data->connid);

				free_data(data);
				break;
			}
			case TYPE_CONNECT_SUCCEED:
			{
				printf("Proxy connection %d connected succesefully\n", data->connid);

				if (pthread_create(&data->client_read, NULL, do_client_read, (void *) data) == -1)
					error("Error %d occured in pthread_create(client_read)", errno);

				if (pthread_create(&data->client_write, NULL, do_client_write, (void *) data) == -1)
					error("Error %d occured in pthread_create(client_write)", errno);
				break;
			}
			case TYPE_CONNECTION_CLOSE:
			{
				printf("Proxy closed connection %d\n", data->connid);

				send_packet(data, TYPE_CONNECTION_CLOSED, NULL, 0);

				free_data(data);
				break;
			}
			case TYPE_PROXY_DATA:
			{
				printf("P->L readed %d bytes for connection %d\n", itp->itun->length, data->connid);

				buffer_add(data->proxy_buffer, itp->data, itp->itun->length);
				break;
			}
		}

		free(itp);
	}

	pthread_exit(NULL);
}

static void do_accept(void)
{
	struct addrinfo hints, *servinfo;
	bzero(&hints, sizeof(struct addrinfo));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(params->bind_address, params->bind_port, &hints, &servinfo) != 0)
		error("Error %d occured in getaddrinfo()", errno);

	int sockfd;
	struct addrinfo *p;
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (sockfd == -1)
			continue;

		if (!set_socket_option(sockfd, SOL_SOCKET, SO_REUSEADDR, 1))
		{
			close(sockfd);
			error("Error %d occured in set_socket_option(sockfd, SO_REUSEADDR)", errno);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo);

	if (p == NULL)
		error("Can't bind to %s:%s", params->bind_address, params->bind_port);

	printf("Waiting for incoming connection at %s:%s\n", params->bind_address, params->bind_port);

	if (!set_socket_option(sockfd, SOL_SOCKET, SO_RCVBUF, SOCKET_BUFFER_SIZE))
		error("Error %d occured in set_socket_option(sockfd, SO_RCVBUF, %d)", errno, SOCKET_BUFFER_SIZE);

	if (!set_socket_option(sockfd, SOL_SOCKET, SO_SNDBUF, SOCKET_BUFFER_SIZE))
		error("Error %d occured in set_socket_option(sockfd, SO_SNDBUF, %d)", errno, SOCKET_BUFFER_SIZE);

	if (listen(sockfd, -1) == -1)
		error("Error %d occured in listen(sockfd)", errno);

	pthread_t proxy_read;

	if (pthread_create(&proxy_read, NULL, do_proxy_read, NULL) == -1)
		error("Error %d occured in pthread_create(proxy_read)", errno);

	while (1)
	{
		int connfd = accept(sockfd, NULL, 0);
		if (connfd == -1)
		{
			if (errno == EINTR || errno == ECONNABORTED)
				continue;
			error("Error %d occured in accept()", errno);
		}

		printf("Client connected\n");

		if (!set_socket_option(connfd, SOL_SOCKET, SO_KEEPALIVE, 1))
			error("Error %d occured in set_socket_option(conn_fd, SO_KEEPALIVE, 1)", errno);

		if (!set_socket_option(connfd, IPPROTO_TCP, TCP_NODELAY, 1))
			error("Error %d occured in set_socket_option(conn_fd, TCP_NODELAY, 1)", errno);

		struct connection_data *data = init_data(connfd);

		if (pthread_create(&data->proxy_write, NULL, do_proxy_write, (void *) data) == -1)
			error("Error %d occured in pthread_create(proxy_write)", errno);

		struct payload_connect payload;
		bzero(&payload, sizeof(struct payload_connect));

		payload.ip	= params->dst_ip;
		payload.port	= atoi(params->proxy_port);

		ring_add(params->connections, data, data->connid);
		send_packet(data, TYPE_CONNECT, &payload, sizeof(struct payload_connect));
	}
}

static void print_usage(void)
{
	printf("usage: itun [-h] [-a bind_address] [-p bind_port] proxy_address proxy_port\n");
}

static void do_init(int argc, char *argv[])
{
	params = (struct init_params *) malloc(sizeof(struct init_params));
	bzero(params, sizeof(struct init_params));

	char opt;
	while ((opt = getopt(argc, argv, "ha:p:i:")) != -1)
	{
		switch (opt)
		{
			case 'a':
			{
				params->bind_address = strdup(optarg);
				break;
			}
			case 'p':
			{
				params->bind_port = strdup(optarg);
				break;
			}
			case 'h':
			{
				print_usage();

				free_params();
				exit(EXIT_SUCCESS);
			}
		}
	}

	if (argc < optind + 2)
	{
		print_usage();

		free_params();
		exit(EXIT_SUCCESS);
	}

	params->proxy_address	= strdup(argv[optind]);
	params->proxy_port	= strdup(argv[optind + 1]);

	if (params->bind_port == NULL)
		params->bind_port = strdup(DEFAULT_PORT);

	params->connections = ring_new();
}

int main(int argc, char *argv[])
{
	srand(time(NULL));

	do_init(argc, argv);

	do_init_libnet();
	do_init_libpcap();

	do_accept();

	do_uninit();

	exit(EXIT_SUCCESS);
}