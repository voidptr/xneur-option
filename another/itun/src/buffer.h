
#ifndef _ITUN_BUFFER_H_
#define _ITUN_BUFFER_H_

#include <pthread.h>

struct buffer_chunk
{
	char *data;
	int size;
	int id;
};

struct itun_buffer
{
	struct buffer_chunk *chunks;
	int chunks_count;

	int next_chunk_id;

	pthread_mutex_t mutex;
};

struct itun_buffer* buffer_new(void);
void buffer_free(struct itun_buffer *buffer);
void buffer_add(struct itun_buffer *buffer, const char *data, int size);

#endif /* _ITUN_BUFFER_H_ */