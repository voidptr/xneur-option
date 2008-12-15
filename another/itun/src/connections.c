
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
	pthread_mutex_lock(&buffer->mutex);

	if (buffer->chunks != NULL)
		free(buffer->chunks);

	pthread_mutex_unlock(&buffer->mutex);
	pthread_mutex_destroy(&buffer->mutex);

	free(buffer);
}

void connections_add(struct connections_buffer *buffer, void *data, int id)
{
	pthread_mutex_lock(&buffer->mutex);

	int lower = 0, upper = buffer->chunks_count - 1;
	while (lower <= upper)
	{
		int cur = (lower + upper) / 2;
		if (buffer->chunks[cur].id > id)
			upper = cur - 1;
		else if (buffer->chunks[cur].id < id)
			lower = cur + 1;
		else
			return;
	}

	buffer->chunks = realloc(buffer->chunks, (buffer->chunks_count + 1) * sizeof(struct connections_chunk));
	buffer->chunks_count++;

	struct connections_chunk *chunk = NULL;

	if (upper < 0)
	{
		memmove(buffer->chunks + 1, buffer->chunks, (buffer->chunks_count - 1) * sizeof(struct connections_chunk));
		chunk = &buffer->chunks[0];
	}
	else if (lower >= buffer->chunks_count)
		chunk = &buffer->chunks[buffer->chunks_count - 1];
	else
	{
		memmove(buffer->chunks + lower + 1, buffer->chunks + lower, (buffer->chunks_count - lower - 1) * sizeof(struct connections_chunk));
		chunk = &buffer->chunks[lower];
	}

	bzero(chunk, sizeof(struct connections_chunk));
	chunk->data	= data;
	chunk->id	= id;

	printf("Added connection %d\n", id);

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
	if (buffer->chunks_count == 0)
	{
		free(buffer->chunks);
		buffer->chunks = NULL;

		pthread_mutex_unlock(&buffer->mutex);
		return;
	}

	if (index != buffer->chunks_count)
		memcpy(buffer->chunks + index, buffer->chunks + index + 1, (buffer->chunks_count - index) * sizeof(struct connections_chunk));
	buffer->chunks = realloc(buffer->chunks, buffer->chunks_count * sizeof(struct connections_chunk));

	printf("Removed connection %d\n", id);

	pthread_mutex_unlock(&buffer->mutex);
}
