
#ifndef _ITUN_PACKETS_H_
#define _ITUN_PACKETS_H_

#define MAX_PACKET_WAIT_TIME	2
#define MAX_PACKET_REQUESTS	5

struct itun_header
{
	int magic;
	int connid;
	int type;
	int seq;
	int length;
};

struct itun_packet
{
	struct itun_header *header;

	char *data;
	void *connection;

	int src_ip;
	int dst_ip;
	int icmp_type;

	int time;
	int requests;
};

struct packets_buffer* packets_new(void);
void packets_free(struct packets_buffer *buffer);
void packets_add(struct packets_buffer *buffer, struct itun_packet *packet);
struct itun_packet* packets_take(struct packets_buffer *buffer);
void packets_free_packet(struct itun_packet *packet);
void packets_drop(struct packets_buffer *buffer, int connid);
void packets_remove(struct packets_buffer *buffer, int seq, int connid);
struct itun_packet* packets_get_expired_packet(struct packets_buffer *buffer, int expire_time);

#endif /* _ITUN_PACKETS_H_ */
