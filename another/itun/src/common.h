
#ifndef _ITUN_COMMON_H_
#define _ITUN_COMMON_H_

#define SOCKET_BUFFER_SIZE	65535
#define MAX_PAYLOAD_SIZE	8096
#define PCAP_BUFFER_SIZE	LIBNET_IPV4_H + LIBNET_ICMPV4_ECHO_H + sizeof(struct itun_packet) + MAX_PAYLOAD_SIZE
#define MAX_TRANSFER_UNIT	1000

#define MAGIC_NUMBER		0x6E757469 // "itun"

#define SHUTDOWN_READ		1
#define SHUTDOWN_WRITE		2

void error(const char *msg, ...);
int set_socket_option(int sockfd, int level, int option, int value);
int add_fcntl(int sockfd, int add_fcntl);

struct itun_packet* parse_packet(unsigned int len, const unsigned char *packet);
void send_icmp_packet(int src_ip, int dst_ip, struct itun_packet *packet);

void do_uninit(void);
void do_init_libpcap(void);
void do_init_libnet(void);

#endif /* _ITUN_COMMON_H_ */
