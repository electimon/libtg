#ifndef TG_QUEUE_H
#define TG_QUEUE_H

#include <stdint.h>
#include "../tg/tg.h"

typedef	struct tg_queue_node_ {
	uint64_t msg_id;
	buf_t msg_data;
	void *on_donep;
	int (*on_done)(void *on_donep, const buf_t data);
	void *chunkp; 
	buf_t (*chunk)(void *chunkp, uint32_t received, uint32_t total);
} tg_queue_node_t;

tg_queue_node_t * 
tg_queue_node_new(uint64_t id, const buf_t data, 
	void *on_donep,
	int (*on_done)(void *userdata, const buf_t data),
	void *chunkp, 
	buf_t (*chunk)(void *chunkp, uint32_t received, uint32_t total));

void tg_queue_node_free(tg_queue_node_t *node);

int tg_socket_daemon(tg_t *tg);

#endif /* ifndef TG_QUEUE_H */
