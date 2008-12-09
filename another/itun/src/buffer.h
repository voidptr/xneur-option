
#ifndef _ITUN_BUFFER_H_
#define _ITUN_BUFFER_H_

struct buffer_chunk
{
	char *data;
	int size;
};

struct itun_buffer* buffer_new(void);
void buffer_free(struct itun_buffer *buffer);
void buffer_add(struct itun_buffer *buffer, const char *data, int size);
struct buffer_chunk* buffer_take(struct itun_buffer *buffer);
void buffer_free_chunk(struct buffer_chunk *chunk);

#endif /* _ITUN_BUFFER_H_ */