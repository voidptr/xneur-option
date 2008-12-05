
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

struct connection_data
{
	struct init_params *params;
	struct itun_buffer *program_buffer;
	struct itun_buffer *proxy_buffer;

	int connfd;
	int connid;
	int last_seq;
};

static void send_connect_packet(struct connection_data *data)
{
	struct itun_packet *packet = malloc(sizeof(struct itun_packet));
	bzero(packet, sizeof(struct itun_packet));

	packet->magic		= MAGIC_NUMBER;
	packet->id		= data->connid;
	packet->ip		= data->params->dst_ip;
	packet->port		= atoi(data->params->proxy_port);
	packet->state		= STATE_NEW_CONNECTION;
	packet->seq		= data->last_seq++;

	send_icmp_packet(data->params, packet, sizeof(struct itun_packet));
}

static struct connection_data* init_data(struct init_params *params, int connfd)
{
	struct connection_data *data = malloc(sizeof(struct connection_data));
	bzero(data, sizeof(struct connection_data));

	data->params		= params;
	data->program_buffer	= buffer_new();
	data->proxy_buffer	= buffer_new();

	data->connfd		= connfd;
	data->connid		= params->last_id++;
	data->last_seq		= 0;

	return data;
}

void* do_proxy_read(void *arg)
{
	arg = arg;

	pthread_exit(NULL);
}

void* do_proxy_write(void *arg)
{
	arg = arg;

	pthread_exit(NULL);
}

void* do_client_write(void *arg)
{
	arg = arg;
/*
	struct connection_data *data	= (struct connection_data *) arg;
	struct init_params *params	= data->params;

	while (1)
	{
		libnet_ptag_t icmp_tag = libnet_build_icmpv4_echo(ICMP_ECHO, 0, 0, 0, 0, NULL, 0, params->libnet, 0);
		if (icmp_tag == -1)
			error("Can't build icmp packet: %s", libnet_geterror(params->libnet));

		libnet_ptag_t ip_tag = libnet_build_ipv4(LIBNET_IPV4_H + LIBNET_ICMPV4_ECHO_H, IPTOS_LOWDELAY | IPTOS_THROUGHPUT, rand(), 0, 128, IPPROTO_ICMP, 0, params->src_ip, params->dst_ip, NULL, 0, params->libnet, 0);
		if (ip_tag == -1)
			error("Can't build ip packet: %s", libnet_geterror(params->libnet));

		int written = libnet_write(params->libnet);
		if (written == -1)
			error("Can't send icmp echo packet: %s", libnet_geterror(params->libnet));

		printf("Sended %d bytes icmp-echo packet\n", written);
	}
*/
	pthread_exit(NULL);
}

void* do_client_read(void *arg)
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

		printf("readed: %d\n", readed);
		if (readed == 0)
			break;

		buffer_add(data->program_buffer, temp_buf, readed);

		temp_buf[readed] = '\0';
		printf("%s", temp_buf);
	}

	free(temp_buf);
	close(data->connfd);

	printf("Disconnected\n");

	pthread_exit(NULL);
}

static void do_accept(struct init_params *params)
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

		struct connection_data *data = init_data(params, connfd);

		pthread_t thread_read, thread_write;

		if (pthread_create(&thread_read, NULL, do_proxy_read, (void *) data) == -1)
			error("Error %d occured in pthread_create(proxy_read)", errno);

		if (pthread_create(&thread_write, NULL, do_proxy_write, (void *) data) == -1)
			error("Error %d occured in pthread_create(proxy_write)", errno);

		send_connect_packet(data);
	}
}

static void print_usage(void)
{
	printf("usage: itun [-h] [-a bind_address] [-p bind_port] proxy_address proxy_port\n");
}

static struct init_params* get_params(int argc, char *argv[])
{
	struct init_params *params = (struct init_params *) malloc(sizeof(struct init_params));
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

				free_params(params);
				exit(EXIT_SUCCESS);
			}
		}
	}

	if (argc < optind + 2)
	{
		print_usage();

		free_params(params);
		exit(EXIT_SUCCESS);
	}

	params->proxy_address	= strdup(argv[optind]);
	params->proxy_port	= strdup(argv[optind + 1]);

	if (params->bind_port == NULL)
		params->bind_port = strdup(DEFAULT_PORT);

	return params;
}

int main(int argc, char *argv[])
{
	srand(time(NULL));

	struct init_params *params = get_params(argc, argv);

	do_init_libnet(params);
	do_init_libpcap(params);

	do_accept(params);

	do_uninit(params);

	exit(EXIT_SUCCESS);
}