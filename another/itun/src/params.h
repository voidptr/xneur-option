
#ifndef _ITUN_PARAMS_H_
#define _ITUN_PARAMS_H_

#include <pthread.h>
#include <libnet.h>
#include <pcap.h>

struct init_params
{
	libnet_t *libnet;
	pcap_t *libpcap;

	pthread_t icmp_parse;
	pthread_t request_packets;

	struct connections_buffer *connections;

	struct packets_buffer *packets_receive;
	struct packets_buffer *packets_send;

	struct data_buffer *client_buffer;

	char *bind_address;
	char *bind_port;

	char *proxy_address;

	char *destination_address;
	char *destination_port;

	int bind_ip;
	int proxy_ip;
	int dest_ip;
};

struct init_params* params_new(void);
void params_free(struct init_params *params);

#endif /* _ITUN_PARAMS_H_ */
