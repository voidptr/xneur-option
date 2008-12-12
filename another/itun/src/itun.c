
#include <sys/socket.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include <getopt.h>

#include "connections.h"
#include "packets.h"
#include "data.h"
#include "common.h"
#include "params.h"

#define DEFAULT_PORT		"8000"

struct init_params *params	= NULL;

pthread_t icmp_read		= 0;
pthread_t icmp_write		= 0;

struct connection_data
{
	struct data_buffer *proxy_buffer;

	int connfd;
	int connid;

	int receive_seq;
	int send_seq;

	pthread_t client_read;
	pthread_t client_write;
};

static struct connection_data* init_connection_data(int connfd)
{
	struct connection_data *data = malloc(sizeof(struct connection_data));
	bzero(data, sizeof(struct connection_data));

	data->proxy_buffer	= data_new();

	data->connfd		= connfd;
	data->connid		= params->last_id++;

	return data;
}

void free_connection_data(struct connection_data *data)
{
	if (data->connfd != 0)
		close(data->connfd);

	if (data->client_read != 0)
		pthread_cancel(data->client_read);
	if (data->client_write != 0)
		pthread_cancel(data->client_write);

	if (data->proxy_buffer != NULL)
		data_free(data->proxy_buffer);

	connections_remove(params->connections, data->connid);
	packets_drop(params->packets_send, data->connid);
	packets_drop(params->packets_receive, data->connid);

	free(data);
}

static void send_packet(struct connection_data *data, int type, void *payload, int size)
{
	struct itun_packet *packet = malloc(sizeof(struct itun_packet));
	bzero(packet, sizeof(struct itun_packet));

	packet->header = malloc(sizeof(struct itun_header));
	bzero(packet->header, sizeof(struct itun_header));

	packet->header->magic		= MAGIC_NUMBER;
	packet->header->connid		= data->connid;
	packet->header->type		= type;
	packet->header->seq		= data->send_seq++;
	packet->header->length		= size;

	packet->connection		= data;
	packet->icmp_type		= ICMP_ECHO;

	if (payload != NULL)
	{
		packet->data = malloc(size * sizeof(char));
		memcpy(packet->data, payload, size);
	}

	packets_add(params->packets_send, packet);
	send_icmp_packet(params->bind_ip, params->proxy_ip, packet);
}

static void* do_client_write(void *arg)
{
	struct connection_data *data = (struct connection_data *) arg;

	while (1)
	{
		struct data_chunk *chunk = data_take(data->proxy_buffer);

		int done = 0;
		while (done != chunk->size)
		{
			int writed = write(data->connfd, chunk->data + done, chunk->size - done);
			if (writed == -1)
			{
				if (errno != EINTR)
					error("Error %d while writing data to client", errno);
				continue;
			}

			if (writed == 0)
				break;

			done += writed;

			printf("\n");
			printf("L->C writed %d bytes to connection %d\n", writed, data->connid);
			printf("%s\n", chunk->data);
		}

		data_free_chunk(chunk);
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
				error("Error %d while reading data from client", errno);
			continue;
		}

		if (readed == 0)
			break;

		temp_buf[readed] = 0;

		printf("\n");
		printf("C->L readed %d bytes for connection %d\n", readed, data->connid);
		printf("%s\n", temp_buf);

		data_add(params->client_buffer, temp_buf, readed, data);
	}

	free(temp_buf);
	close(data->connfd);

	printf("Connection with client %d closed\n", data->connid);
	send_packet(data, TYPE_CONNECTION_CLOSED, NULL, 0);

	pthread_exit(NULL);
}

static void* do_icmp_write(void *arg)
{
	if (arg) {}

	while (1)
	{
		struct data_chunk *chunk = data_take(params->client_buffer);

		struct connection_data *data = (struct connection_data *) chunk->connection;

		printf("L->I writed %d bytes to connection %d\n", chunk->size, data->connid);

		send_packet(data, TYPE_CLIENT_DATA, chunk->data, chunk->size);
		data_free_chunk(chunk);
	}

	pthread_exit(NULL);
}

static void* do_icmp_parse(void *arg)
{
	if (arg) {}

	while (1)
	{
		struct itun_packet *packet	= packets_take(params->packets_receive);
		struct connection_data *data	= (struct connection_data *) packet->connection;

		switch (packet->header->type)
		{
			case TYPE_CONNECT_FAILED:
			{
				printf("Proxy connection %d failed to connect with server\n", data->connid);

				free_connection_data(data);
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

				free_connection_data(data);
				break;
			}
			case TYPE_PROXY_DATA:
			{
				printf("I->L readed %d bytes for connection %d\n", packet->header->length, data->connid);

				data_add(data->proxy_buffer, packet->data, packet->header->length, data);
				break;
			}
		}

		packets_free_packet(packet);
	}

	pthread_exit(NULL);
}

static void* do_icmp_read(void *arg)
{
	if (arg) {}

	struct pcap_pkthdr *header;
	const unsigned char *packet;
	int result;

	while ((result = pcap_next_ex(params->libpcap, &header, &packet)) >= 0)
	{
		if (result == 0)
			continue;

		struct itun_packet *itp = parse_packet(header->len, packet);
		if (itp == NULL)
		{
			printf("Wrong packet received, skipping\n");
			continue;
		}

		if (itp->icmp_type == ICMP_ECHOREPLY)
		{
			if ((itp->header->type & PROXY_FLAG) == PROXY_FLAG)
				continue;

			printf("Received reply to packet %d for connection %d\n", itp->header->seq, itp->header->connid);
			packets_remove(params->packets_send, itp->header->seq, itp->header->connid);
			continue;
		}

		if ((itp->header->type & PROXY_FLAG) != PROXY_FLAG)
			continue;

		struct connection_data *connection = (struct connection_data *) connections_get(params->connections, itp->header->connid);
		if (connection == NULL)
		{
			packets_free_packet(itp);
			continue;
		}

		itp->connection = connection;
		packets_add(params->packets_receive, itp);
	}

	pthread_exit(NULL);
}

static void* do_request_packets(void *arg)
{
	if (arg) {}

	while (1)
	{
		while (1)
		{
			struct itun_packet *packet = packets_get_expired_packet(params->packets_send, MAX_PACKET_WAIT_TIME);
			if (packet == NULL)
				break;

			if (packet->requests <= MAX_PACKET_REQUESTS)
			{
				printf("Resend packet with seq %d fo connection %d\n", packet->header->seq, packet->header->connid);
				send_icmp_packet(params->bind_ip, params->proxy_ip, packet);
				continue;
			}

			struct connection_data *data = (struct connection_data *) packet->connection;
			if (data == NULL)
				continue;

			printf("Reached %d packet with seq %d loss, closing connection %d\n", packet->requests, packet->header->seq, data->connid);
			free_connection_data(data);
		}

		usleep(1000);
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
			error("Error %d occured in set_socket_option(sockfd, SO_REUSEADDR)", errno);

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

	if (pthread_create(&icmp_read, NULL, do_icmp_read, NULL) == -1)
		error("Error %d occured in pthread_create(icmp_read)", errno);

	if (pthread_create(&icmp_write, NULL, do_icmp_write, NULL) == -1)
		error("Error %d occured in pthread_create(icmp_write)", errno);

	if (pthread_create(&params->icmp_parse, NULL, do_icmp_parse, NULL) == -1)
		error("Error %d occured in pthread_create(icmp_parse)", errno);

	if (pthread_create(&params->request_packets, NULL, do_request_packets, NULL) == -1)
		error("Error %d occured in pthread_create(request_packets)", errno);

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

		struct connection_data *data = init_connection_data(connfd);

		struct itun_packet_connect ipc;
		bzero(&ipc, sizeof(struct itun_packet_connect));

		ipc.ip		= params->dest_ip;
		ipc.port	= atoi(params->destination_port);

		connections_add(params->connections, data, data->connid);
		send_packet(data, TYPE_CONNECT, &ipc, sizeof(struct itun_packet_connect));
	}
}

static void print_usage(void)
{
	printf("usage: itun [-h] [-a bind_address] [-p bind_port] proxy_address destination_address destination_port\n");
}

static void do_init(int argc, char *argv[])
{
	params = params_new();

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

				params_free(params);
				exit(EXIT_SUCCESS);
			}
		}
	}

	if (argc < optind + 3)
	{
		print_usage();

		params_free(params);
		exit(EXIT_SUCCESS);
	}

	if (params->bind_port == NULL)
		params->bind_port = strdup(DEFAULT_PORT);

	params->proxy_address		= strdup(argv[optind]);

	params->destination_address	= strdup(argv[optind + 1]);
	params->destination_port	= strdup(argv[optind + 2]);
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
