
#include "common.h"

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

void send_icmp_packet(struct init_params *params, void *data, int size)
{
	pthread_mutex_lock(&libnet_mutex);

	libnet_ptag_t icmp_tag = libnet_build_icmpv4_echo(ICMP_ECHO, 0, 0, 0, 0, data, size, params->libnet, 0);
	if (icmp_tag == -1)
		error("Can't build icmp packet: %s", libnet_geterror(params->libnet));

	libnet_ptag_t ip_tag = libnet_build_ipv4(LIBNET_IPV4_H + LIBNET_ICMPV4_ECHO_H + size, IPTOS_LOWDELAY | IPTOS_THROUGHPUT, rand(), 0, 128, IPPROTO_ICMP, 0, params->src_ip, params->dst_ip, NULL, 0, params->libnet, 0);
	if (ip_tag == -1)
		error("Can't build ip packet: %s", libnet_geterror(params->libnet));

	int written = libnet_write(params->libnet);
	if (written == -1)
		error("Can't send icmp echo packet: %s", libnet_geterror(params->libnet));

	pthread_mutex_unlock(&libnet_mutex);
}

int set_socket_option(int sockfd, int level, int option, int value)
{
	return (setsockopt(sockfd, level, option, &value, sizeof(value)) != -1);
}

void free_params(struct init_params *params)
{
	if (params->bind_address != NULL)
		free(params->bind_address);
	if (params->bind_port != NULL)
		free(params->bind_port);
	if (params->proxy_address != NULL)
		free(params->proxy_address);
	if (params->proxy_port != NULL)
		free(params->proxy_port);
	if (params->libnet != NULL)
		libnet_destroy(params->libnet);
	if (params->libpcap != NULL)
		pcap_close(params->libpcap);
	free(params);
}

void do_init_libnet(struct init_params *params)
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

void do_init_libpcap(struct init_params *params)
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

	params->libpcap = pcap_open_live(bind_device, PCAP_BUFFER_SIZE, 0, 1000, errbuf);
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

void do_uninit(struct init_params *params)
{
	pthread_mutex_destroy(&libnet_mutex);
	free_params(params);
}