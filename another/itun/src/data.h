
#ifndef _ITUN_DATA_H_
#define _ITUN_DATA_H_

struct data_chunk
{
	char *data;
	int size;

	void *connection;
};

struct data_buffer* data_new(void);
void data_free(struct data_buffer *buffer);
void data_add(struct data_buffer *buffer, const char *data, int size, void *connection);
struct data_chunk* data_take(struct data_buffer *buffer);
void data_free_chunk(struct data_chunk *chunk);

#endif /* _ITUN_DATA_H_ */
