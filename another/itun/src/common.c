
#include <net/ethernet.h>
#include <netinet/ip_icmp.h>

#include "params.h"
#include "connections.h"
#include "packets.h"

#include "common.h"

#define PCAP_FILTER		"icmp"

extern struct init_params *params;

pthread_mutex_t libnet_mutex;

void error(const char *msg, ...)
{
	va_list params;
	va_start(params, msg);

	vprintf(msg, params);
	printf("\n");

	va_end(params);

	exit(EXIT_FAILURE);
}

int set_socket_option(int sockfd, int level, int option, int value)
{
	return (setsockopt(sockfd, level, option, &value, sizeof(value)) != -1);
}

int add_fcntl(int sockfd, int add_fcntl)
{
	int fcntls = fcntl(sockfd, F_GETFL, 0);
	if (fcntls == -1)
		return 0;

	return (fcntl(sockfd, F_SETFL, fcntls | add_fcntl) != -1);
}

struct itun_packet* parse_packet(unsigned int len, const unsigned char *packet)
{
	if (len < sizeof(struct ether_header))
		return NULL;

	struct ether_header *eth = (struct ether_header *) packet;
	if (ntohs(eth->ether_type) != ETHERTYPE_IP)
		return NULL;

	len	= len - sizeof(struct ether_header);
	packet	= packet + sizeof(struct ether_header);

	if (len < sizeof(struct iphdr))
		return NULL;

	struct iphdr *ip = (struct iphdr *) packet;

	if (ip->version != IPVERSION || ip->protocol != IPPROTO_ICMP)
		return NULL;

	len	= len - ip->ihl * 4;
	packet	= packet + ip->ihl * 4;

	if (len < sizeof(struct icmphdr))
		return NULL;

	struct icmphdr *icmp = (struct icmphdr *) packet;

	len	= len - sizeof(struct icmphdr);
	packet	= packet + sizeof(struct icmphdr);

	if (len < sizeof(struct itun_header))
		return NULL;

	struct itun_header *header = (struct itun_header *) packet;

	if (header->magic != MAGIC_NUMBER)
		return NULL;

	len	= len - sizeof(struct itun_header);
	packet	= packet + sizeof(struct itun_header);

	if (len != (unsigned int) header->length)
		return NULL;

	struct itun_packet *info = malloc(sizeof(struct itun_packet));
	bzero(info, sizeof(struct itun_packet));

	info->icmp_type	= icmp->type;
	info->src_ip	= ip->saddr;
	info->dst_ip	= ip->daddr;

	info->header = malloc(sizeof(struct itun_header));
	memcpy(info->header, header, sizeof(struct itun_header));

	if (len != 0)
	{
		info->data = malloc(len * sizeof(char));
		memcpy(info->data, packet, len * sizeof(char));
	}

	return info;
}

void send_icmp_packet(int src_ip, int dst_ip, struct itun_packet *packet)
{
	pthread_mutex_lock(&libnet_mutex);

	int size = sizeof(struct itun_header) + packet->header->length;

	char *data = malloc(size * sizeof(char));
	memcpy(data, packet->header, sizeof(struct itun_header));
	memcpy(data + sizeof(struct itun_header), packet->data, packet->header->length * sizeof(char));

	libnet_ptag_t icmp_tag = libnet_build_icmpv4_echo(ICMP_ECHO, 0, 0, 0, 0, (unsigned char *) data, size, params->libnet, 0);
	if (icmp_tag == -1)
	{
		free(data);
		error("Can't build icmp packet: %s", libnet_geterror(params->libnet));
	}

	free(data);

	libnet_ptag_t ip_tag = libnet_build_ipv4(LIBNET_IPV4_H + LIBNET_ICMPV4_ECHO_H + sizeof(struct itun_header) + packet->header->length, 0, rand(), 0, 64, IPPROTO_ICMP, 0, src_ip, dst_ip, NULL, 0, params->libnet, 0);
	if (ip_tag == -1)
		error("Can't build ip packet: %s", libnet_geterror(params->libnet));

	int writed = libnet_write(params->libnet);
	if (writed == -1)
		error("Can't send icmp echo packet: %s", libnet_geterror(params->libnet));

	libnet_clear_packet(params->libnet);

	pthread_mutex_unlock(&libnet_mutex);
}

void do_init_libnet(void)
{
	char errbuf[LIBNET_ERRBUF_SIZE];

	pthread_mutex_init(&libnet_mutex, NULL);

	params->libnet = libnet_init(LIBNET_RAW4, params->bind_address, errbuf);
	if (params->libnet == NULL)
		error("Failed to call libnet_init: %s", errbuf);

	if (params->bind_address != NULL)
		params->src_ip = libnet_name2addr4(params->libnet, params->bind_address, LIBNET_RESOLVE);
	else
		params->src_ip = libnet_get_ipaddr4(params->libnet);

	if (params->src_ip == -1)
		error("Can't resolve bind address %s: %s", params->bind_address, libnet_geterror(params->libnet));

	if (params->proxy_address != NULL)
	{
		params->dst_ip = libnet_name2addr4(params->libnet, params->proxy_address, LIBNET_RESOLVE);
		if (params->dst_ip == -1)
			error("Can't resolve proxy address %s: %s", params->proxy_address, libnet_geterror(params->libnet));
	}

	if (params->bind_address != NULL)
		free(params->bind_address);

	struct in_addr addr = {params->src_ip};
	params->bind_address = strdup(inet_ntoa(addr));

	printf("Binded to ip %s\n", params->bind_address);
}

void do_init_libpcap(void)
{
	char errbuf[PCAP_ERRBUF_SIZE];

	pcap_if_t *alldevs;
	if (pcap_findalldevs(&alldevs, errbuf) == -1)
		error("Failed to pcap_findalldevs: %s", errbuf);

	char *bind_device = NULL;
	for (pcap_if_t *device = alldevs; device != NULL; device = device->next)
	{
		for (pcap_addr_t *address = device->addresses; address != NULL; address = address->next)
		{
			if (address->addr == NULL)
				continue;

			struct sockaddr_in *addr = (struct sockaddr_in *) address->addr;
			if (addr->sin_addr.s_addr == (u_int) params->src_ip)
			{
				printf("Found device %s for ip %s\n", device->name, params->bind_address);
				bind_device = strdup(device->name);
				break;
			}
		}
	}

	pcap_freealldevs(alldevs);

	if (bind_device == NULL)
		error("Failed to find device for ip %s", params->bind_address);

	params->libpcap = pcap_open_live(bind_device, PCAP_BUFFER_SIZE, 0, 10000, errbuf);
	if (params->libpcap == NULL)
		error("Failed to pcap_open_live(%s): %s", bind_device, errbuf);

	struct bpf_program fp;
	if (pcap_compile(params->libpcap, &fp, PCAP_FILTER, 0, 0) == -1)
		error("Can't compile pcap filter");

	if (pcap_setfilter(params->libpcap, &fp) == -1)
		error("Can't set pcap filter");

	pcap_setdirection(params->libpcap, PCAP_D_IN);

	printf("Capturing incoming packets at %s\n", bind_device);
}

void do_uninit(void)
{
	pthread_mutex_destroy(&libnet_mutex);
	params_free(params);
}
