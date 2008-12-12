
#include <sys/socket.h>
#include <sys/select.h>

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

struct init_params *params	= NULL;
pthread_t icmp_write		= 0;

struct connection_data
{
	struct data_buffer *client_buffer;

	int connfd;
	int connid;

	int receive_seq;
	int send_seq;

	int client_ip;
	int server_ip;
	int server_port;

	pthread_t server_connect;
	pthread_t server_read;
	pthread_t server_write;
};

static struct connection_data* init_connection_data(struct itun_packet *packet, int connfd)
{
	struct itun_packet_connect *ipc = (struct itun_packet_connect *) packet->data;

	struct connection_data *data = malloc(sizeof(struct connection_data));
	bzero(data, sizeof(struct connection_data));

	data->client_buffer	= data_new();

	data->connfd		= connfd;
	data->connid		= packet->header->connid;

	data->client_ip		= packet->dst_ip;
	data->server_ip		= ipc->ip;
	data->server_port	= ipc->port;

	return data;
}

static void free_connection_data(struct connection_data *data)
{
	if (data->connfd != 0)
		close(data->connfd);

	if (data->server_connect != 0)
		pthread_cancel(data->server_connect);
	if (data->server_read != 0)
		pthread_cancel(data->server_read);
	if (data->server_write != 0)
		pthread_cancel(data->server_write);

	if (data->client_buffer != NULL)
		data_free(data->client_buffer);

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

	if (payload != NULL)
	{
		packet->data = malloc(size * sizeof(char));
		memcpy(packet->data, payload, size);
	}

	packets_add(params->packets_send, packet);
	send_icmp_packet(params->bind_ip, data->client_ip, packet);
}

static void* do_server_write(void *arg)
{
	struct connection_data *data = (struct connection_data *) arg;

	while (1)
	{
		struct data_chunk *chunk = data_take(data->client_buffer);

		int done = 0;
		while (done != chunk->size)
		{
			int writed = write(data->connfd, chunk->data + done, chunk->size - done);
			if (writed == -1)
			{
				if (errno != EINTR)
					error("Error %d while writing data to server", errno);
				continue;
			}

			if (writed == 0)
				break;

			done += writed;

			printf("\n");
			printf("L->S writed %d bytes for connection %d\n", writed, data->connid);
			printf("%s\n", chunk->data);
		}

		data_free_chunk(chunk);
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
				error("Error %d while reading data from server", errno);
			continue;
		}

		if (readed == 0)
			break;

		temp_buf[readed] = 0;

		printf("\n");
		printf("S->L readed %d bytes for connection %d\n", readed, data->connid);
		printf("%s\n", temp_buf);

		data_add(params->client_buffer, temp_buf, readed, data);
	}

	free(temp_buf);
	close(data->connfd);

	printf("Connection with server %d closed\n", data->connid);
	send_packet(data, TYPE_CONNECTION_CLOSE, NULL, 0);

	pthread_exit(NULL);
}

static void* do_server_connect(void *arg)
{
	struct connection_data *data = (struct connection_data *) arg;

	int connfd = socket(AF_INET, SOCK_STREAM, 0);
	if (connfd == -1)
		error("Can't open new socket");

	data->connfd = connfd;

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
		free_connection_data(data);
		pthread_exit(NULL);
	}

	connections_add(params->connections, data, data->connid);
	send_packet(data, TYPE_CONNECT_SUCCEED, NULL, 0);

	data->server_connect = 0;

	if (pthread_create(&data->server_read, NULL, do_server_read, (void *) data) == -1)
		error("Error %d occured in pthread_create(server_read)", errno);

	if (pthread_create(&data->server_write, NULL, do_server_write, (void *) data) == -1)
		error("Error %d occured in pthread_create(server_write)", errno);

	pthread_exit(NULL);
}

static void* do_icmp_write(void *arg)
{
	if (arg) {}

	while (1)
	{
		struct data_chunk *chunk = data_take(params->client_buffer);

		struct connection_data *data = (struct connection_data *) chunk->connection;

		printf("L->I writed %d bytes for connection %d\n", chunk->size, data->connid);

		send_packet(data, TYPE_PROXY_DATA, chunk->data, chunk->size);
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
			case TYPE_CONNECT:
			{
				struct in_addr addr = {packet->src_ip};
				char *from_ip = inet_ntoa(addr);

				printf("Received connect packet from %s\n", from_ip);

				if (packet->header->length != sizeof(struct itun_packet_connect))
				{
					printf("Packet seems to be fragmented, skipping\n");
					break;
				}

				struct connection_data *data = init_connection_data(packet, 0);

				if (pthread_create(&data->server_connect, NULL, do_server_connect, (void *) data) == -1)
					error("Error %d occured in pthread_create(server_connect)", errno);
				break;
			}
			case TYPE_CLIENT_DATA:
			{
				printf("I->L readed %d bytes for connection %d\n", packet->header->length, data->connid);

				data_add(data->client_buffer, packet->data, packet->header->length, data);
				break;
			}
			case TYPE_CONNECTION_CLOSED:
			{
				printf("Client connecion %d closed succesefully\n", data->connid);
				free_connection_data(data);
				break;
			}
		}

		packets_free_packet(packet);
	}
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

			struct connection_data *data = (struct connection_data *) packet->connection;
			if (data == NULL)
				continue;

			if (packet->requests <= MAX_PACKET_REQUESTS)
			{
				printf("Resend packet with seq %d fo connection %d\n", packet->header->seq, packet->header->connid);
				send_icmp_packet(params->bind_ip, data->client_ip, packet);
				continue;
			}

			printf("Reached %d packet with seq %d loss, closing connection %d\n", packet->requests, packet->header->seq, data->connid);
			free_connection_data(data);
		}

		usleep(1000);
	}

	pthread_exit(NULL);
}

static void do_accept(void)
{
	if (pthread_create(&icmp_write, NULL, do_icmp_write, NULL) == -1)
		error("Error %d occured in pthread_create(icmp_write)", errno);

	if (pthread_create(&params->icmp_parse, NULL, do_icmp_parse, NULL) == -1)
		error("Error %d occured in pthread_create(icmp_parse)", errno);

	if (pthread_create(&params->request_packets, NULL, do_request_packets, NULL) == -1)
		error("Error %d occured in pthread_create(request_packets)", errno);

	printf("Waiting for incoming packets\n");

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
			if ((itp->header->type & PROXY_FLAG) != PROXY_FLAG)
				continue;

			printf("Received reply to packet %d for connection %d\n", itp->header->seq, itp->header->connid);
			packets_remove(params->packets_send, itp->header->seq, itp->header->connid);
			continue;
		}

		if ((itp->header->type & PROXY_FLAG) == PROXY_FLAG)
			continue;

		struct connection_data *connection = NULL;
		if (itp->header->type != TYPE_CONNECT)
		{
			connection = (struct connection_data *) connections_get(params->connections, itp->header->connid);
			if (connection == NULL)
				continue;
		}

		itp->connection = connection;
		packets_add(params->packets_receive, itp);
	}

	printf("Breaked\n");
}

static void print_usage(void)
{
	printf("usage: itunp [-h] [-a bind_address]\n");
}

static void do_init(int argc, char *argv[])
{
	params = params_new();

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

				params_free(params);
				exit(EXIT_SUCCESS);
			}
		}
	}
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
