
#include <sys/socket.h>
#include <net/ethernet.h>
#include <netinet/ip_icmp.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include <getopt.h>

#include "buffer.h"
#include "common.h"

struct init_params *params = NULL;

struct connection_data
{
	struct itun_buffer *client_buffer;
	struct itun_buffer *server_buffer;

	int connfd;
	int connid;

	int receive_seq;
	int send_seq;

	int client_ip;
	int server_ip;
	int server_port;

	pthread_t client_write;
	pthread_t server_read;
	pthread_t server_write;
};

static struct connection_data* init_data(struct itun_packet *packet, int connfd)
{
	struct payload_connect *payload = (struct payload_connect *) packet->data;

	struct connection_data *data = malloc(sizeof(struct connection_data));
	bzero(data, sizeof(struct connection_data));

	data->client_buffer	= buffer_new();
	data->server_buffer	= buffer_new();

	data->connfd		= connfd;
	data->connid		= packet->itun->id;

	data->client_ip		= packet->dst_ip;
	data->server_ip		= payload->ip;
	data->server_port	= payload->port;

	return data;
}

static void free_data(struct connection_data *data)
{
	if (data->connfd != 0)
		close(data->connfd);
	if (data->server_read != 0)
		pthread_cancel(data->server_read);
	if (data->server_write != 0)
		pthread_cancel(data->server_write);
	if (data->client_write != 0)
		pthread_cancel(data->client_write);
	if (data->server_buffer != NULL)
		buffer_free(data->server_buffer);
	if (data->client_buffer != NULL)
		buffer_free(data->client_buffer);
	free(data);
}

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

	send_icmp_packet(params->src_ip, data->client_ip, packet);

	free(packet);
}

static void* do_client_write(void *arg)
{
	struct connection_data *data = (struct connection_data *) arg;

	while (1)
	{
		struct buffer_chunk *chunk = buffer_take(data->server_buffer);

		printf("L->C writed %d bytes for connection %d\n", chunk->size, data->connid);

		send_packet(data, TYPE_PROXY_DATA, chunk->data, chunk->size);
		buffer_free_chunk(chunk);
	}

	pthread_exit(NULL);
}

static void* do_server_read(void *arg)
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

		printf("S->L readed %d bytes for connection %d\n", readed, data->connid);
		printf("%s\n", temp_buf);

		buffer_add(data->server_buffer, temp_buf, readed);
	}

	free(temp_buf);
	close(data->connfd);

	printf("Connection with server %d closed\n", data->connid);
	send_packet(data, TYPE_CONNECTION_CLOSE, NULL, 0);

	pthread_exit(NULL);
}

static void* do_server_write(void *arg)
{
	struct connection_data *data = (struct connection_data *) arg;

	while (1)
	{
		struct buffer_chunk *chunk = buffer_take(data->client_buffer);

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
			printf("L->S writed %d bytes to connection %d\n", writed, data->connid);
		}

		buffer_free_chunk(chunk);
	}

	pthread_exit(NULL);
}

static void* do_server_connect(void *arg)
{
	struct itun_packet *packet = (struct itun_packet *) arg;

	int connfd = socket(AF_INET, SOCK_STREAM, 0);
	if (connfd == -1)
		error("Can't open new socket");

	struct connection_data *data = init_data(packet, connfd);

	struct sockaddr_in addr;
	bzero(&addr, sizeof(struct sockaddr_in));

	addr.sin_family		= AF_INET;
	addr.sin_port		= htons(data->server_port);
	addr.sin_addr.s_addr	= data->server_ip;

	struct in_addr saddr = {data->server_ip};
	printf("Connecting to %s:%d\n", inet_ntoa(saddr), data->server_port);

	if (connect(data->connfd, (struct sockaddr *) &addr, sizeof(struct sockaddr)) != 0)
	{
		printf("Can't connect to %s:%d\n", inet_ntoa(saddr), data->server_port);

		send_packet(data, TYPE_CONNECT_FAILED, NULL, 0);
		free_data(data);
		pthread_exit(NULL);
	}

	ring_add(params->connections, data, data->connid);
	send_packet(data, TYPE_CONNECT_SUCCEED, NULL, 0);

	if (pthread_create(&data->server_write, NULL, do_server_write, (void *) data) == -1)
		error("Error %d occured in pthread_create(server_write)", errno);

	if (pthread_create(&data->server_read, NULL, do_server_read, (void *) data) == -1)
		error("Error %d occured in pthread_create(server_read)", errno);

	if (pthread_create(&data->client_write, NULL, do_client_write, (void *) data) == -1)
		error("Error %d occured in pthread_create(client_write)", errno);

	pthread_exit(NULL);
}

static void print_usage(void)
{
	printf("usage: itunp [-h] [-a bind_address]\n");
}

static void do_accept(void)
{
	printf("Waiting for incoming packets\n");

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
			if ((itp->itun->type & PROXY_FLAG) != PROXY_FLAG)
				continue;

			printf("Received reply to packet %d for connection %d\n", itp->itun->seq, itp->itun->id);
			continue;
		}

		if ((itp->itun->type & PROXY_FLAG) == PROXY_FLAG)
			continue;

		packets_add(itp);

		struct connection_data *data = NULL;
		if (itp->itun->type != TYPE_CONNECT)
			data = (struct connection_data *) ring_get(params->connections, itp->itun->id);

		switch (itp->itun->type)
		{
			case TYPE_CONNECT:
			{
				struct in_addr addr = {itp->src_ip};
				char *from_ip = inet_ntoa(addr);

				printf("Received connect packet from %s\n", from_ip);

				if (itp->itun->length != sizeof(struct payload_connect))
				{
					printf("Packet seems to be fragmented, skipping\n");
					break;
				}

				pthread_t thread_connect;

				if (pthread_create(&thread_connect, NULL, do_server_connect, (void *) itp) == -1)
					error("Error %d occured in pthread_create(server_connect)", errno);

				break;
			}
			case TYPE_CLIENT_DATA:
			{
				printf("C->L readed %d bytes for connection %d\n", itp->itun->length, data->connid);

				buffer_add(data->client_buffer, itp->data, itp->itun->length);
				break;
			}
			case TYPE_CONNECTION_CLOSED:
			{
				printf("Client connecion %d closed succesefully\n", data->connid);
				free_data(data);
				break;
			}
		}

		free(itp);
	}

	printf("Breaked\n");
}

static void do_init(int argc, char *argv[])
{
	params = (struct init_params *) malloc(sizeof(struct init_params));
	bzero(params, sizeof(struct init_params));

	char opt;
	while ((opt = getopt(argc, argv, "ha:")) != -1)
	{
		switch (opt)
		{
			case 'a':
			{
				params->bind_address = strdup(optarg);
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