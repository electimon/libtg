#ifndef TG_QUEUE_H
#define TG_QUEUE_H

#include <stdint.h>
#include "../tg/tg.h"

typedef	struct tg_queue_node_ {
	buf_t msg;
	uint64_t msgid;
	void *on_donep;
	int (*on_done)(void *on_donep, const buf_t data);
	void *chunkp; 
	buf_t (*chunk)(void *chunkp, uint32_t received, uint32_t total);
} tg_queue_node_t;

tg_queue_node_t * 
tg_queue_node_new(const buf_t msg, 
	uint64_t msgid,
	void *on_donep,
	int (*on_done)(void *userdata, const buf_t data),
	void *chunkp, 
	buf_t (*chunk)(void *chunkp, uint32_t received, uint32_t total));

void tg_queue_node_free(tg_queue_node_t *node);

int tg_start_send_queue_manager(tg_t *tg);
int tg_start_receive_queue_manager(tg_t *tg);

#endif /* ifndef TG_QUEUE_H */
