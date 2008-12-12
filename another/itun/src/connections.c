
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>

#include <pthread.h>

#include "connections.h"

struct connections_chunk
{
	void *data;
	int id;
};

struct connections_buffer
{
	struct connections_chunk *chunks;
	int chunks_count;

	pthread_mutex_t mutex;
};

static int connections_get_index(struct connections_buffer *buffer, int id)
{
	int lower = 0, upper = buffer->chunks_count - 1;
	while (lower <= upper)
	{
		int cur = (lower + upper) / 2;
		if (buffer->chunks[cur].id > id)
			upper = cur - 1;
		else if (buffer->chunks[cur].id < id)
			lower = cur + 1;
		else
			return cur;
	}

	printf("Not found connections chunk %d\n", id);
	return -1;
}

struct connections_buffer* connections_new(void)
{
	struct connections_buffer *buffer = malloc(sizeof(struct connections_buffer));
	bzero(buffer, sizeof(struct connections_buffer));

	pthread_mutex_init(&buffer->mutex, NULL);

	return buffer;
}

void connections_free(struct connections_buffer *buffer)
{
	pthread_mutex_destroy(&buffer->mutex);

	if (buffer->chunks != NULL)
		free(buffer->chunks);
	free(buffer);
}

void connections_add(struct connections_buffer *buffer, void *data, int id)
{
	pthread_mutex_lock(&buffer->mutex);

	buffer->chunks = realloc(buffer->chunks, (buffer->chunks_count + 1) * sizeof(struct connections_chunk));

	struct connections_chunk *chunk = &buffer->chunks[buffer->chunks_count];
	bzero(chunk, sizeof(struct connections_chunk));

	chunk->data	= data;
	chunk->id	= id;

	buffer->chunks_count++;

	printf("Added connection %d at pos %d\n", id, buffer->chunks_count - 1);

	pthread_mutex_unlock(&buffer->mutex);
}

void* connections_get(struct connections_buffer *buffer, int id)
{
	pthread_mutex_lock(&buffer->mutex);

	void *data = NULL;

	int index = connections_get_index(buffer, id);
	if (index != -1)
		data = buffer->chunks[index].data;
		
	pthread_mutex_unlock(&buffer->mutex);

	return data;
}

void connections_remove(struct connections_buffer *buffer, int id)
{
	pthread_mutex_lock(&buffer->mutex);

	int index = connections_get_index(buffer, id);
	if (index == -1)
	{
		pthread_mutex_unlock(&buffer->mutex);
		return;
	}

	buffer->chunks_count--;

	if (index != buffer->chunks_count)
	{
		memcpy(buffer->chunks + index, buffer->chunks + index + 1, (buffer->chunks_count - index) * sizeof(struct connections_chunk));
		buffer->chunks = realloc(buffer->chunks, buffer->chunks_count * sizeof(struct connections_chunk));
	}
	else
	{
		free(buffer->chunks);
		buffer->chunks = NULL;
	}

	printf("Removed connection %d from pos %d\n", id, index);

	pthread_mutex_unlock(&buffer->mutex);
}
