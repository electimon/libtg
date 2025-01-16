#ifndef TG_QUEUE_H
#define TG_QUEUE_H
#include "list.h"
#include "tg.h"
#include <stdint.h>

typedef struct tg_queue_{
	tg_t *tg;
	char ip[16];
	int port;
	pthread_t p;
	int socket;
	bool loop;
	buf_t query;
	uint64_t msgid;
	pthread_mutex_t m;
	void *userdata;
	void (*on_done)(void *userdata, const tl_t *tl);
	void *progressp;
	int (*progress)(void *progressp, int size, int total);
} tg_queue_t;

tg_queue_t * tg_queue_new(
		tg_t *tg, buf_t *query, 
		const char *ip, int port,
		void *userdata, void (*on_done)(void *userdata, const tl_t *tl),
		void *progressp, 
		int (*progress)(void *progressp, int size, int total));

int tg_queue_send(tg_t *tg, buf_t *query, 
		void *userdata, void (*callback)(void *userdata, tl_t *tl));

void tg_queue_cancell_all(tg_t *tg);
#endif /* ifndef TG_QUEUE_H */
