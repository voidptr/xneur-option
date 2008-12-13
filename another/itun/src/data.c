
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <errno.h>

#include <pthread.h>
#include <semaphore.h>

#include "connections.h"
#include "data.h"

struct data_buffer
{
	struct data_chunk **chunks;
	int chunks_count;

	pthread_mutex_t mutex;
	sem_t avail;
};

struct data_buffer* data_new(void)
{
	struct data_buffer *buffer = malloc(sizeof(struct data_buffer));
	bzero(buffer, sizeof(struct data_buffer));

	pthread_mutex_init(&buffer->mutex, NULL);
	sem_init(&buffer->avail, 0, 0);

	return buffer;
}

void data_free_chunk(struct data_chunk *chunk)
{
	if (chunk->data != NULL)
		free(chunk->data);
	free(chunk);
}

void data_free(struct data_buffer *buffer)
{
	pthread_mutex_lock(&buffer->mutex);
	sem_destroy(&buffer->avail);

	for (int i = 0; i < buffer->chunks_count; i++)
		data_free_chunk(buffer->chunks[i]);

	if (buffer->chunks != NULL)
		free(buffer->chunks);

	pthread_mutex_unlock(&buffer->mutex);
	pthread_mutex_destroy(&buffer->mutex);

	free(buffer);
}

void data_add(struct data_buffer *buffer, const char *data, int size, void *connection)
{
	pthread_mutex_lock(&buffer->mutex);

	struct data_chunk *chunk = malloc(sizeof(struct data_chunk));
	bzero(chunk, sizeof(struct data_chunk));

	if (data != NULL)
	{
		chunk->data = malloc(size * sizeof(char));
		memcpy(chunk->data + chunk->size, data, size * sizeof(char));
	}

	chunk->size		= size;
	chunk->connection	= connection;

	buffer->chunks = realloc(buffer->chunks, (buffer->chunks_count + 1) * sizeof(struct data_chunk *));
	buffer->chunks_count++;

	buffer->chunks[buffer->chunks_count - 1] = chunk;

	sem_post(&buffer->avail);

	pthread_mutex_unlock(&buffer->mutex);
}

struct data_chunk* data_take(struct data_buffer *buffer)
{
	if (sem_wait(&buffer->avail) != 0)
		return NULL;
	pthread_mutex_lock(&buffer->mutex);

	if (buffer->chunks_count == 0)
	{
		printf("Can't found chunk while sem val is > 0\n");
		pthread_mutex_unlock(&buffer->mutex);
		return NULL;
	}

	struct data_chunk *chunk = buffer->chunks[0];

	buffer->chunks_count--;

	if (buffer->chunks_count != 0)
	{
		memcpy(buffer->chunks, buffer->chunks + 1, buffer->chunks_count * sizeof(struct data_chunk *));
		buffer->chunks = realloc(buffer->chunks, buffer->chunks_count * sizeof(struct data_chunk *));
	}
	else
	{
		free(buffer->chunks);
		buffer->chunks = NULL;
	}
	
	pthread_mutex_unlock(&buffer->mutex);

	return chunk;
}
