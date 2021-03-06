
#include <sys/types.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
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

	if (len < LIBNET_IPV4_H)
		return NULL;

	struct ip *iph = (struct ip *) packet;

	if (iph->ip_v != IPVERSION || iph->ip_p != IPPROTO_ICMP)
		return NULL;

	len	= len - iph->ip_hl * 4;
	packet	= packet + iph->ip_hl * 4;

	if (len < sizeof(struct icmphdr))
		return NULL;

	struct icmp *icmph = (struct icmp *) packet;

	len	= len - LIBNET_ICMPV4_ECHO_H;
	packet	= packet + LIBNET_ICMPV4_ECHO_H;

	if (len < LIBNET_ICMPV4_ECHO_H)
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

	info->icmp_type	= icmph->icmp_type;
	info->client_ip	= iph->ip_src.s_addr;

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

	if (packet->data != NULL)
	{
		libnet_ptag_t packet_data_tag = libnet_build_data((unsigned char *) packet->data, packet->header->length, params->libnet, 0);
		if (packet_data_tag == -1)
			error("Can't build data packet: %s", libnet_geterror(params->libnet));
	}

	libnet_ptag_t packet_header_tag = libnet_build_data((unsigned char *) packet->header, sizeof(struct itun_header), params->libnet, 0);
	if (packet_header_tag == -1)
		error("Can't build data packet: %s", libnet_geterror(params->libnet));

	libnet_ptag_t icmp_tag = libnet_build_icmpv4_echo(packet->icmp_type, 0, 0, rand(), 0, NULL, 0, params->libnet, 0);
	if (icmp_tag == -1)
		error("Can't build icmp packet: %s", libnet_geterror(params->libnet));

	libnet_ptag_t ip_tag = libnet_build_ipv4(LIBNET_IPV4_H + LIBNET_ICMPV4_ECHO_H + sizeof(struct itun_header) + packet->header->length, 0, rand(), 0, 64, IPPROTO_ICMP, 0, src_ip, dst_ip, NULL, 0, params->libnet, 0);
	if (ip_tag == -1)
		error("Can't build ip packet: %s", libnet_geterror(params->libnet));

	int writed = libnet_write(params->libnet);
	if (writed == -1)
		error("Can't send icmp echo packet: %s", libnet_geterror(params->libnet));

	struct in_addr addr = {dst_ip};
	printf("Sended packet to %s\n", inet_ntoa(addr));

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
		params->bind_ip = libnet_name2addr4(params->libnet, params->bind_address, LIBNET_RESOLVE);
	else
		params->bind_ip = libnet_get_ipaddr4(params->libnet);

	if (params->bind_ip == -1)
		error("Can't resolve bind address %s: %s", params->bind_address, libnet_geterror(params->libnet));

	if (params->proxy_address != NULL)
	{
		params->proxy_ip = libnet_name2addr4(params->libnet, params->proxy_address, LIBNET_RESOLVE);
		if (params->proxy_ip == -1)
			error("Can't resolve proxy address %s: %s", params->proxy_address, libnet_geterror(params->libnet));
	}

	if (params->destination_address != NULL)
	{
		params->dest_ip = libnet_name2addr4(params->libnet, params->destination_address, LIBNET_RESOLVE);
		if (params->dest_ip == -1)
			error("Can't resolve destination address %s: %s", params->destination_address, libnet_geterror(params->libnet));
	}

	if (params->bind_address != NULL)
		free(params->bind_address);

	struct in_addr addr = {params->bind_ip};
	params->bind_address = strdup(inet_ntoa(addr));
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
			if (addr->sin_addr.s_addr == (unsigned int) params->bind_ip)
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
