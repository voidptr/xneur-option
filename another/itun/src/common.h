
#ifndef _ITUN_COMMON_H_
#define _ITUN_COMMON_H_

#include <pthread.h>
#include <libnet.h>
#include <pcap.h>

#define PCAP_FILTER		"icmp"

#define SOCKET_BUFFER_SIZE	65535
#define PAYLOAD_SIZE		8096
#define PCAP_BUFFER_SIZE	LIBNET_IPV4_H + LIBNET_ICMPV4_ECHO_H + sizeof(struct itun_packet) + PAYLOAD_SIZE

#define MAGIC_NUMBER		0x6E757469 // "itun"

enum itun_states
{
	STATE_NEW_CONNECTION = 0,
};

struct itun_packet
{
	int magic;
	int id;
	int ip;
	int port;
	enum itun_states state;
	int seq;
	int ack;
	int length;
};

struct init_params
{
	libnet_t *libnet;
	pcap_t *libpcap;

	char *bind_address;
	char *bind_port;

	char *proxy_address;
	char *proxy_port;

	int src_ip;
	int dst_ip;

	int last_id;
};

void error(const char *msg, ...);
void send_icmp_packet(struct init_params *params, void *data, int size);
void do_init_libpcap(struct init_params *params);
int set_socket_option(int sockfd, int level, int option, int value);
void free_params(struct init_params *params);
void do_init_libnet(struct init_params *params);
void do_uninit(struct init_params *params);
#endif /* _ITUN_COMMON_H_ */
