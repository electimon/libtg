#include "queue.h"
#include "../tl/alloc.h"
#include "../tg/list.h"
#include "net.h"
#include <stdio.h>
#include <unistd.h>

tg_queue_node_t *
tg_queue_node_new(const buf_t msg, 
	void *on_donep,
	int (*on_done)(void *userdata, const buf_t data),
	void *chunkp, 
	buf_t (*chunk)(void *chunkp, uint32_t received, uint32_t total))
{
	tg_queue_node_t *n = NEW(tg_queue_node_t, {
			perror("malloc");
			return NULL;
			});

	n->msg = buf_add_buf(msg);
	n->on_donep = on_donep;
	n->on_done = on_done;
	n->chunkp = chunkp;
	n->chunk = chunk;
	return n;
}

void tg_queue_node_free(tg_queue_node_t *node)
{
	if (node){
		buf_free(node->msg);
		free(node);
	}
}

static void * _tg_socket_datemon(void * data)
{
	tg_t *tg = data;
	ON_LOG(tg, "%s: start", __func__);

	while (tg->queue_manager) {
		tg_net_send_queue_node(tg);
		usleep(100000); // in microseconds
	}

	pthread_exit(0);	
}

int tg_start_queue_manager(tg_t *tg){

	tg->queue_manager = 1;

	// start thread
	if (pthread_create(
			&(tg->sync_dialogs_tid), 
			NULL, 
			_tg_socket_datemon, 
			tg))
	{
		ON_ERR(tg, "%s: can't create thread", __func__);
		return 1;
	}

	return 0;
}
