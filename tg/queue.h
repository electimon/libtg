#ifndef TG_QUEUE_H
#define TG_QUEUE_H
#include "list.h"
#include "tg.h"
#include <stdint.h>

typedef struct tg_queue_{
	tg_t *tg;
	pthread_t p;
	int socket;
	bool loop;
	bool catched;
	buf_t query;
	uint64_t msgid;
	void *userdata;
	void (*on_done)(void *userdata, const tl_t *tl);
} tg_queue_t;

tg_queue_t * tg_queue_new(
		tg_t *tg, buf_t *query, 
		void *userdata, void (*on_done)(void *userdata, const tl_t *tl));
int tg_queue_send(tg_t *tg, buf_t *query, 
		void *userdata, void (*callback)(void *userdata, tl_t *tl));

#endif /* ifndef TG_QUEUE_H */
