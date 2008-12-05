
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

struct connection_data
{
	struct init_params *params;
	struct itun_buffer *program_buffer;
	struct itun_buffer *proxy_buffer;

	int connfd;
	int connid;
	int last_seq;

	int src_ip;
	int dst_ip;
	int dst_port;
};

static struct connection_data* init_data(struct init_params *params, int src_ip, struct itun_packet *packet)
{
	struct connection_data *data = malloc(sizeof(struct connection_data));
	bzero(data, sizeof(struct connection_data));

	data->params		= params;
	data->program_buffer	= buffer_new();
	data->proxy_buffer	= buffer_new();

	data->connfd		= 0;
	data->connid		= packet->id;
	data->last_seq		= 0;
	data->src_ip		= src_ip;
	data->dst_ip		= packet->ip;
	data->dst_port		= packet->port;

	return data;
}

void* do_client_read(void *arg)
{
	arg = arg;

	pthread_exit(NULL);
}

void* do_client_write(void *arg)
{
	arg = arg;

	pthread_exit(NULL);
}

void* do_server_write(void *arg)
{
	arg = arg;

	pthread_exit(NULL);
}

void* do_server_read(void *arg)
{
	arg = arg;

	pthread_exit(NULL);
}

static void print_usage(void)
{
	printf("usage: itun [-h] [-a bind_address]\n");
}

static struct init_params* get_params(int argc, char *argv[])
{
	struct init_params *params = (struct init_params *) malloc(sizeof(struct init_params));
	bzero(params, sizeof(struct init_params));

	char opt;
	while ((opt = getopt(argc, argv, "ha:i:")) != -1)
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

				free_params(params);
				exit(EXIT_SUCCESS);
			}
		}
	}

	return params;
}

static void do_connect(struct connection_data *data)
{
	struct in_addr addr = {data->dst_ip};
	printf("Connecting to server %s:%d\n", inet_ntoa(addr), data->dst_port);
}

static void do_accept(struct init_params *params)
{
	printf("Waiting for incoming packets\n");

	struct pcap_pkthdr *header;
	const u_char *packet;
	int result;

	while ((result = pcap_next_ex(params->libpcap, &header, &packet)) >= 0)
	{
		if (result == 0)
			continue;

		uint32_t len = header->len;
		if (len < sizeof(struct ether_header))
			continue;

		struct ether_header *eth = (struct ether_header *) packet;
		if (ntohs(eth->ether_type) != ETHERTYPE_IP)
			continue;

		len	= len - sizeof(struct ether_header);
		packet	= packet + sizeof(struct ether_header);

		if (len < sizeof(struct iphdr))
			continue;

		struct iphdr *ip = (struct iphdr *) packet;

		if (ip->version != IPVERSION || ip->protocol != IPPROTO_ICMP)
			continue;

		len	= len - ip->ihl * 4;
		packet	= packet + ip->ihl * 4;

		if (len < sizeof(struct icmphdr))
			continue;

		struct icmphdr *icmp = (struct icmphdr *) packet;

		len	= len - sizeof(struct icmphdr);
		packet	= packet + sizeof(struct icmphdr);

		if (len < sizeof(struct itun_packet))
			continue;

		struct itun_packet *itun = (struct itun_packet *) packet;

		if (itun->magic != MAGIC_NUMBER)
			continue;

		if (icmp->type == ICMP_ECHOREPLY)
		{
			printf("Received reply to packet %d:%d\n", itun->id, itun->seq);
			continue;
		}

		struct in_addr addr = {ip->daddr};
		printf("Received incoming client packet from %s\n", inet_ntoa(addr));

		if (itun->state == STATE_NEW_CONNECTION)
		{
			struct connection_data *data = init_data(params, ip->daddr, itun);
			do_connect(data);
			continue;
		}

		len	= len - sizeof(struct itun_packet);
		packet	= packet + sizeof(struct itun_packet);
	}
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