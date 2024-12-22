#include "queue.h"
#include "../tl/alloc.h"
#include "../tg/list.h"
#include "net.h"
#include <stdio.h>
#include <unistd.h>

tg_queue_node_t *
tg_queue_node_new(const buf_t msg, 
	uint64_t msgid,
	void *on_donep,
	int (*on_done)(void *userdata, const buf_t data),
	void *chunkp, 
	buf_t (*chunk)(void *chunkp, uint32_t received, uint32_t total))
{
	printf("%s\n", __func__);
	tg_queue_node_t *n = NEW(tg_queue_node_t, {
			perror("malloc");
			return NULL;
			});

	n->msg = buf_add_buf(msg);
	n->msgid =msgid;
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

static void * _tg_send_daemon(void * data)
{
	tg_t *tg = data;
	ON_LOG(tg, "%s: start", __func__);

	while (tg->send_queue_manager) {
		tg_net_send_queue_node(tg);
		usleep(100000); // in microseconds
	}

	pthread_exit(0);	
}

int tg_start_send_queue_manager(tg_t *tg){

	tg->send_queue_manager = 1;

	// start thread
	if (pthread_create(
				&(tg->send_queue_tid), 
			NULL, 
			_tg_send_daemon, 
			tg))
	{
		ON_ERR(tg, "%s: can't create thread", __func__);
		return 1;
	}

	return 0;
}

static void * _tg_receive_daemon(void * data)
{
	tg_t *tg = data;
	ON_LOG(tg, "%s: start", __func__);

	while (tg->receive_queue_manager) {
		// receive
		usleep(100000); // in microseconds
		int sockfd = tg_net_open(tg);
		if (sockfd < 0)
			continue;
		tg_net_queue_receive(tg, sockfd);
	}

	pthread_exit(0);	
}

int tg_start_receive_queue_manager(tg_t *tg){

	tg->receive_queue_manager = 1;

	// start thread
	if (pthread_create(
			&(tg->receive_queue_tid), 
			NULL, 
			_tg_receive_daemon, 
			tg))
	{
		ON_ERR(tg, "%s: can't create thread", __func__);
		return 1;
	}

	return 0;
}
