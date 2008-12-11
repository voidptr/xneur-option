
#ifndef _ITUN_CONNECTIONS_H_
#define _ITUN_CONNECTIONS_H_

struct connections_buffer* connections_new(void);
void connections_free(struct connections_buffer *buffer);
void connections_add(struct connections_buffer *buffer, void *data, int id);
void* connections_get(struct connections_buffer *buffer, int id);
void connections_remove(struct connections_buffer *buffer, int id);

#endif /* _ITUN_CONNECTIONS_H_ */