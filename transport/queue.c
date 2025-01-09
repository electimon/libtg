#include "queue.h"
#include "../tl/alloc.h"
#include "../tg/list.h"
#include "net.h"
#include <stdio.h>
#include <unistd.h>
#include "../tg/tg.h"
#include "transport.h"
#include <sys/socket.h>

static tl_t *receive_tl(tg_t *tg, int sockfd)
{
	ON_LOG(tg, "%s", __func__);
	buf_t r = buf_new();
	// get length of the package
	uint32_t len;
	recv(sockfd, &len, 4, 0);
	ON_LOG(tg, "%s: prepare to receive len: %d", __func__, len);
	if (len < 0) {
		// this is error - report it
		ON_ERR(tg, "%s: received wrong length: %d", __func__, len);
		tg_net_close(tg, sockfd);
		buf_free(r);
		return NULL;
	}

	// realloc buf to be enough size
	if (buf_realloc(&r, len)){
		// handle error
		ON_ERR(tg, "%s: error buf realloc to size: %d", __func__, len);
		tg_net_close(tg, sockfd);
		buf_free(r);
		return NULL;
	}

	// get data
	uint32_t received = 0; 
	while (received < len){
		int s = 
			recv(sockfd, &r.data[received], len - received, 0);	
		if (s<0){
			ON_ERR(tg, "%s: socket error: %d", __func__, s);
			buf_free(r);
			return NULL;
		}
		ON_LOG(tg, "%s: received chunk: %d", __func__, s);

		received += s;
		
		// ask to send new chunk
		if (received < len){
			ON_LOG(tg, "%s: expected size: %d, received: %d (%d%%)", 
					__func__, len, received, received*100/len);
			
			// receive more data
			continue;
		}
		break;
	}

	ON_LOG(tg, "%s: expected size: %d, received: %d (%d%%)", 
			__func__, len, received, received*100/len);

	if (received < len){
		// some error
		ON_ERR(tg, "%s: can't receive data", __func__);
		tg_net_close(tg, sockfd);
		buf_free(r);
		return NULL;
	}

	// get payload 
	r.size = len;
	if (r.size == 4 && buf_get_ui32(r) == 0xfffffe6c){
		buf_free(r);
		tg_net_close(tg, sockfd);
		return NULL;
	}

	buf_t d = tg_decrypt(tg, r, true);
	if (!d.size){
		buf_free(r);
		tg_net_close(tg, sockfd);
		return NULL;
	}
	buf_free(r);

	buf_t msg = tg_deheader(tg, d, true);
	if (!msg.size){
		buf_free(d);
		tg_net_close(tg, sockfd);
		return NULL;
	}
	buf_free(d);

	// deserialize 
	return tg_deserialize(tg, &msg);
}

static void receive_in_queue(tg_t *tg, int sockfd)
{
	ON_LOG(tg, "%s", __func__);
	tl_t *tl = receive_tl(tg, sockfd);
	if (tl == NULL)
		return;

	tg_run_api_async_receive(tg, tl);
}

static void * _tg_receive_daemon(void * data)
{
	tg_t *tg = data;
	ON_LOG(tg, "%s", __func__);

	while (tg->receive_queue_manager) {
		// receive
		ON_LOG(tg, "%s: receive...", __func__);
		usleep(100000); // in microseconds
		//int sockfd = tg_net_open(tg);
		//tg->sockfd = sockfd;
		int sockfd = tg->queue_sockfd;
		if (sockfd < 0)
			continue;
		receive_in_queue(tg, sockfd);
	}

	pthread_exit(0);	
}

int tg_start_receive_queue_manager(tg_t *tg){

	ON_LOG(tg, "%s", __func__);
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
