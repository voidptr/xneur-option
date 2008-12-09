
#include <stdlib.h>
#include <strings.h>
#include <string.h>

#include <pthread.h>
#include <semaphore.h>

#include "buffer.h"

//xx
#include <stdio.h>

#define NORMAL_BUFFER_SIZE	8096

struct itun_buffer
{
	struct buffer_chunk **chunks;
	int chunks_count;

	pthread_mutex_t mutex;
	sem_t avail;
};

static struct buffer_chunk* add_chunk(struct itun_buffer *buffer)
{
	buffer->chunks = realloc(buffer->chunks, (buffer->chunks_count + 1) * sizeof(struct buffer_chunk *));

	struct buffer_chunk *chunk = malloc(sizeof(struct buffer_chunk));
	bzero(chunk, sizeof(struct buffer_chunk));

	buffer->chunks[buffer->chunks_count] = chunk;

	buffer->chunks_count++;
	sem_post(&buffer->avail);

	return chunk;
}

void buffer_free_chunk(struct buffer_chunk *chunk)
{
	if (chunk->data != NULL)
		free(chunk->data);
	free(chunk);
}

struct itun_buffer* buffer_new(void)
{
	struct itun_buffer *buffer = malloc(sizeof(struct itun_buffer));
	bzero(buffer, sizeof(struct itun_buffer));

	pthread_mutex_init(&buffer->mutex, NULL);
	sem_init(&buffer->avail, 0, 0);

	return buffer;
}

void buffer_free(struct itun_buffer *buffer)
{
	sem_destroy(&buffer->avail);
	pthread_mutex_destroy(&buffer->mutex);

	for (int i = 0; i < buffer->chunks_count; i++)
		buffer_free_chunk(buffer->chunks[i]);

	if (buffer->chunks != NULL)
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
		chunk = buffer->chunks[buffer->chunks_count - 1];
		if (chunk->size + size > NORMAL_BUFFER_SIZE * 2)
			chunk = add_chunk(buffer);
	}

	chunk->data = realloc(chunk->data, (chunk->size + size) * sizeof(char));
	memcpy(chunk->data + chunk->size, data, size * sizeof(char));

	chunk->size += size;

	pthread_mutex_unlock(&buffer->mutex);
}

struct buffer_chunk* buffer_take(struct itun_buffer *buffer)
{
	printf("Waiting for semaphore\n");
	sem_wait(&buffer->avail);
	printf("Semaphore waited\n");

	pthread_mutex_lock(&buffer->mutex);

	if (buffer->chunks_count == 0)
	{
		printf("Can't found chunk while sem val is > 0\n");
		pthread_mutex_unlock(&buffer->mutex);
		return NULL;
	}

	struct buffer_chunk *chunk = buffer->chunks[0];

	buffer->chunks_count--;

	if (buffer->chunks_count != 0)
		memcpy(buffer->chunks, buffer->chunks + 1, buffer->chunks_count * sizeof(struct buffer_chunk *));
	buffer->chunks = realloc(buffer->chunks, buffer->chunks_count * sizeof(struct buffer_chunk *));

	printf("Shifted chunk from buffer\n");

	pthread_mutex_unlock(&buffer->mutex);

	return chunk;
}