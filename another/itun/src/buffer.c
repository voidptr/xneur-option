
#include <stdlib.h>
#include <strings.h>
#include <string.h>

#include "buffer.h"

#define NORMAL_BUFFER_SIZE	2048

static struct buffer_chunk* add_chunk(struct itun_buffer *buffer)
{
	buffer->chunks_count++;
	buffer->chunks = realloc(buffer->chunks, buffer->chunks_count * sizeof(struct buffer_chunk));

	struct buffer_chunk *chunk = &(buffer->chunks[buffer->chunks_count - 1]);
	chunk->id = buffer->next_chunk_id++;

	return chunk;
}

struct itun_buffer* buffer_new(void)
{
	struct itun_buffer *buffer = malloc(sizeof(struct itun_buffer));
	bzero(buffer, sizeof(struct itun_buffer));

	buffer->next_chunk_id = rand();

	pthread_mutex_init(&buffer->mutex, NULL);

	return buffer;
}

void buffer_free(struct itun_buffer *buffer)
{
	pthread_mutex_destroy(&buffer->mutex);

	for (int i = 0; i < buffer->chunks_count; i++)
	{
		if (buffer->chunks[i].data == NULL)
			continue;
		free(buffer->chunks[i].data);
	}
	free(buffer->chunks);
	free(buffer);
}

void buffer_add(struct itun_buffer *buffer, const char *data, int size)
{
	pthread_mutex_lock(&buffer->mutex);

	struct buffer_chunk *chunk;
	if (buffer->chunks == NULL || buffer->chunks_count == 0)
		chunk = add_chunk(buffer);
	else
	{
		chunk = &(buffer->chunks[buffer->chunks_count - 1]);
		if (chunk->size + size > NORMAL_BUFFER_SIZE * 2)
			chunk = add_chunk(buffer);
	}

	chunk->data = realloc(chunk->data, (chunk->size + size) * sizeof(char));
	memcpy(chunk->data + chunk->size, data, size * sizeof(char));
	chunk->size += size;

	pthread_mutex_unlock(&buffer->mutex);
}