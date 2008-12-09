
#ifndef _ITUN_COMMON_H_
#define _ITUN_COMMON_H_

#include <pthread.h>
#include <libnet.h>
#include <pcap.h>

#include "ring.h"

#define SOCKET_BUFFER_SIZE	65535
#define PAYLOAD_SIZE		8096
#define PCAP_BUFFER_SIZE	LIBNET_IPV4_H + LIBNET_ICMPV4_ECHO_H + sizeof(struct itun_packet) + PAYLOAD_SIZE

#define MAGIC_NUMBER		0x6E757469 // "itun"
#define PROXY_FLAG		1 << 31

struct payload_connect
{
	int ip;
	int port;
};

enum itun_types
{
	TYPE_CONNECTED			= 1 | PROXY_FLAG,
	TYPE_CONNECT_FAILED,
	TYPE_CONNECT_SUCCEED,
	TYPE_CONNECTION_CLOSE,
	TYPE_PROXY_DATA,

	TYPE_CONNECT			= 1,
	TYPE_CONNECTION_CLOSED,
	TYPE_CLIENT_DATA,
};

struct itun_header
{
	int magic;
	int id;
	int type;
	int seq;
	int length;
};

struct itun_packet
{
	struct itun_header *itun;

	int src_ip;
	int dst_ip;
	int icmp_type;

	char *data;
};

struct init_params
{
	libnet_t *libnet;
	pcap_t *libpcap;
	struct ring_buffer *connections;

	char *bind_address;
	char *bind_port;

	char *proxy_address;
	char *proxy_port;

	int src_ip;
	int dst_ip;

	int last_id;
};

void error(const char *msg, ...);
void free_params(void);

struct itun_packet* parse_packet(u_int len, const u_char *packet);
void send_icmp_packet(int src_ip, int dst_ip, struct itun_header *packet);
int set_socket_option(int sockfd, int level, int option, int value);

void do_uninit(void);
void do_init_libpcap(void);
void do_init_libnet(void);

#endif /* _ITUN_COMMON_H_ */
