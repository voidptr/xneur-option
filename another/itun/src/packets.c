
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <time.h>

#include <pthread.h>
#include <semaphore.h>

#include "packets.h"

struct packets_chunk
{
	struct itun_packet **packets;
	int packets_count;

	int connid;
	int next_seq;
};

struct packets_buffer
{
	struct packets_chunk *chunks;
	int chunks_count;

	pthread_mutex_t mutex;
	sem_t avail;
};

struct packets_buffer* packets_new(void)
{
	struct packets_buffer *buffer = malloc(sizeof(struct packets_buffer));
	bzero(buffer, sizeof(struct packets_buffer));

	pthread_mutex_init(&buffer->mutex, NULL);
	sem_init(&buffer->avail, 0, 0);

	return buffer;
}

static struct packets_chunk* packets_add_chunk(struct packets_buffer *buffer, int connid)
{
	buffer->chunks = realloc(buffer->chunks, (buffer->chunks_count + 1) * sizeof(struct packets_chunk));
	buffer->chunks_count++;

	struct packets_chunk *chunk = &(buffer->chunks[buffer->chunks_count - 1]);
	bzero(chunk, sizeof(struct packets_chunk));

	chunk->connid = connid;

	return chunk;
}

static int packets_find_chunk_index(struct packets_buffer *buffer, int connid)
{
	int lower = 0, upper = buffer->chunks_count - 1;
	while (lower <= upper)
	{
		int cur = (lower + upper) / 2;
		if (buffer->chunks[cur].connid > connid)
			upper = cur - 1;
		else if (buffer->chunks[cur].connid < connid)
			lower = cur + 1;
		else
			return cur;
	}

	return -1;
}

static struct packets_chunk* packets_find_chunk(struct packets_buffer *buffer, int connid)
{
	int index = packets_find_chunk_index(buffer, connid);
	if (index == -1)
		return NULL;

	return &(buffer->chunks[index]);
}

static int packets_find_packet_index(struct packets_chunk *chunk, int seq)
{
	int lower = 0, upper = chunk->packets_count - 1;
	while (lower <= upper)
	{
		int cur		= (lower + upper) / 2;
		int cur_seq	= chunk->packets[cur]->header->seq;

		if (cur_seq > seq)
			upper = cur - 1;
		else if (cur_seq < seq)
			lower = cur + 1;
		else
			return cur;
	}

	return -1;
}

static void packets_add_packet(struct packets_chunk *chunk, struct itun_packet *packet)
{
	int seq = packet->header->seq;

	int lower = 0, upper = chunk->packets_count - 1;
	while (lower <= upper)
	{
		int cur		= (lower + upper) / 2;
		int cur_seq	= chunk->packets[cur]->header->seq;

		if (cur_seq > seq)
			upper = cur - 1;
		else if (cur_seq < seq)
			lower = cur + 1;
		else
		{
			printf("Found packet with same seq %d in packets queue with connecton id %d\n", seq, chunk->connid);
			return;
		}
	}

	chunk->packets = realloc(chunk->packets, (chunk->packets_count + 1) * sizeof(struct itun_packet *));
	chunk->packets_count++;

	if (upper < 0)
	{
		memmove(chunk->packets + 1, chunk->packets, (chunk->packets_count - 1) * sizeof(struct itun_packet *));
		chunk->packets[0] = packet;
	}
	else if (lower >= chunk->packets_count)
		chunk->packets[chunk->packets_count - 1] = packet;
	else
	{
		memmove(chunk->packets + lower + 1, chunk->packets + lower, (chunk->packets_count - lower - 1) * sizeof(struct itun_packet *));
		chunk->packets[lower] = packet;
	}
}

static void packets_shift(struct packets_chunk *chunk)
{
	chunk->packets_count--;

	if (chunk->packets_count != 0)
	{
		memcpy(chunk->packets, chunk->packets + 1, chunk->packets_count * sizeof(struct itun_packet *));
		chunk->packets = realloc(chunk->packets, chunk->packets_count * sizeof(struct itun_packet *));
	}
	else
	{
		free(chunk->packets);
		chunk->packets = NULL;
	}
}

static void packets_free_chunk(struct packets_chunk *chunk)
{
	for (int i = 0; i < chunk->packets_count; i++)
		packets_free_packet(chunk->packets[i]);
	free(chunk->packets);
}

void packets_free(struct packets_buffer *buffer)
{
	sem_destroy(&buffer->avail);
	pthread_mutex_destroy(&buffer->mutex);

	if (buffer->chunks != NULL)
		free(buffer->chunks);
	free(buffer);
}

void packets_add(struct packets_buffer *buffer, struct itun_packet *packet)
{
	pthread_mutex_lock(&buffer->mutex);

	int connid = packet->header->connid;

	struct packets_chunk *chunk = packets_find_chunk(buffer, connid);
	if (chunk == NULL)
		chunk = packets_add_chunk(buffer, connid);

	int packet_seq = packet->header->seq;
	if (packet_seq < chunk->next_seq)
	{
		printf("Received packet with old seq %d \n", packet_seq);
		pthread_mutex_unlock(&buffer->mutex);
		return;
	}

	packet->time = time(NULL);

	printf("Adding packet with seq %d\n", packet_seq);

	packets_add_packet(chunk, packet);
	if (packet_seq == chunk->next_seq)
		sem_post(&buffer->avail);

	pthread_mutex_unlock(&buffer->mutex);
}

struct itun_packet* packets_take(struct packets_buffer *buffer)
{
	sem_wait(&buffer->avail);
	pthread_mutex_lock(&buffer->mutex);

	struct itun_packet *packet = NULL;
	for (int i = 0; i < buffer->chunks_count; i++)
	{
		struct packets_chunk *chunk = &(buffer->chunks[i]);
		if (chunk->packets_count == 0)
			continue;

		packet = chunk->packets[0];
		if (chunk->next_seq != packet->header->seq)
			continue;

		chunk->next_seq++;

		packets_shift(chunk);
		break;
	}

	if (packet == NULL)
		printf("Can't found next packet while sem val is > 0\n");

	pthread_mutex_unlock(&buffer->mutex);

	return packet;
}

void packets_free_packet(struct itun_packet *packet)
{
	if (packet->header != NULL)
		free(packet->header);
	if (packet->data != NULL)
		free(packet->data);
	free(packet);
}

void packets_drop(struct packets_buffer *buffer, int connid)
{
	pthread_mutex_lock(&buffer->mutex);

	int index = packets_find_chunk_index(buffer, connid);
	if (index == -1)
	{
		pthread_mutex_unlock(&buffer->mutex);
		return;
	}

	packets_free_chunk(&(buffer->chunks[index]));

	buffer->chunks_count--;
	if (index != buffer->chunks_count)
		memcpy(buffer->chunks + index, buffer->chunks + index + 1, (buffer->chunks_count - index) * sizeof(struct packets_chunk *));
	buffer->chunks = realloc(buffer->chunks, buffer->chunks_count * sizeof(struct packets_chunk *));

	pthread_mutex_unlock(&buffer->mutex);
}

void packets_remove(struct packets_buffer *buffer, int seq, int connid)
{
	pthread_mutex_lock(&buffer->mutex);

	struct packets_chunk *chunk = packets_find_chunk(buffer, connid);
	if (chunk == NULL)
	{
		pthread_mutex_unlock(&buffer->mutex);
		return;
	}

	int index = packets_find_packet_index(chunk, seq);
	if (index == -1)
	{
		pthread_mutex_unlock(&buffer->mutex);
		return;
	}

	printf("Removing packet with seq %d at pos %d\n", seq, index);

	chunk->packets_count--;
	if (index != chunk->packets_count)
		memcpy(chunk->packets + index, chunk->packets + index + 1, (chunk->packets_count - index) * sizeof(struct itun_packet *));
	chunk->packets = realloc(chunk->packets, chunk->packets_count * sizeof(struct itun_packet *));

	pthread_mutex_unlock(&buffer->mutex);
}

struct itun_packet* packets_get_expired_packet(struct packets_buffer *buffer, int expire_time)
{
	pthread_mutex_lock(&buffer->mutex);

	int cur_time = time(NULL);

	for (int i = 0; i < buffer->chunks_count; i++)
	{
		struct packets_chunk *chunk = &(buffer->chunks[i]);
		for (int j = 0; j < chunk->packets_count; j++)
		{
			struct itun_packet *packet = chunk->packets[j];

			if (cur_time - packet->time < expire_time)
				continue;

			printf("Found expired packet with seq %d and time %d (%d)\n", packet->header->seq, packet->time, cur_time);

			packet->requests++;
			packet->time = cur_time;

			pthread_mutex_unlock(&buffer->mutex);
			return packet;
		}
	}

	pthread_mutex_unlock(&buffer->mutex);
	return NULL;
}