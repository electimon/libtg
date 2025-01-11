#include "queue.h"
#include "../mtx/include/api.h"
#include "../mtx/include/buf.h"
#include "../mtx/include/setup.h"
#include "../mtx/include/types.h"
#include "net.h"
#include "transport.h"
#include "../tl/alloc.h"
#include "list.h"
#include "tg.h"
#include "updates.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#if INTPTR_MAX == INT32_MAX
    #define THIS_IS_32_BIT_ENVIRONMENT
		#define _LD_ "%lld"
#elif INTPTR_MAX == INT64_MAX
    #define THIS_IS_64_BIT_ENVIRONMENT
		#define _LD_ "%ld"
#else
    #error "Environment not 32 or 64-bit."
#endif

enum RTL{
	RTL_EX, // exit loop
	RTL_RQ, // read socket again
	RTL_RS, // resend query
};

static int cmp_msgid(void *msgidp, void *itemp)
{
	tg_queue_t *item = itemp;
	uint64_t *msgid = msgidp;
	if (*msgid == item->msgid)
		return 1;
	return 0;
}

static tg_queue_t *tg_get_queue(tg_t *tg, uint64_t *msgid)
{
	ON_LOG(tg, "%s", __func__);
	int err = pthread_mutex_lock(&tg->queuem);
	if (err){
		ON_ERR(tg, "%s: can't lock mutex: %d", __func__, err);
		return NULL;
	}
	tg_queue_t *q = list_cut(
				&tg->queue, 
				msgid, 
				cmp_msgid);
	pthread_mutex_unlock(&tg->queuem);
	return q;
}

static void catched_tl(tg_queue_t *queue, tl_t *tl)
{
	if (tl == NULL){
		ON_ERR(queue->tg, "%s: tl is NULL", __func__);
		return;
	}
	ON_LOG(queue->tg, "%s: %s", __func__, 
			TL_NAME_FROM_ID(tl->_id));

	switch (tl->_id) {
		case id_gzip_packed:
			{
				// handle gzip
				tl_gzip_packed_t *obj =
					(tl_gzip_packed_t *)tl;

				buf_t buf;
				int _e = gunzip_buf(&buf, obj->packed_data_);
				if (_e)
				{
					char *err = gunzip_buf_err(_e);
					ON_ERR(queue->tg, "%s: %s", __func__, err);
					free(err);
				}
				tl_t *ttl = tl_deserialize(&buf);
				buf_free(buf);
				if (queue->on_done)
					queue->on_done(queue->userdata, ttl);
				// stop query
				queue->loop = false;
				if (ttl)
					tl_free(ttl);
				return;
			}
			break;
		case id_bad_msg_notification:
			{
				tl_bad_msg_notification_t *obj = 
					(tl_bad_msg_notification_t *)tl;
				// handle bad msg notification
				char *err = tg_strerr(tl);
				ON_ERR(queue->tg, "%s", err);
				free(err);
				tl = NULL;
			}
			break;
		case id_rpc_error:
			{
				tl_rpc_error_t *rpc_error = 
					(tl_rpc_error_t *)tl;
				char *err = tg_strerr(tl);
				ON_ERR(queue->tg, "%s: %s", __func__, err);
				free(err);
				tl = NULL;
			}
			break;
	}

	if (queue->on_done)
		queue->on_done(queue->userdata, tl);
	// stop query
	queue->loop = false;
}

static void handle_tl(tg_queue_t *queue, tl_t *tl)
{
	int i;
	if (tl == NULL){
		ON_ERR(queue->tg, "%s: tl is NULL", __func__);
		return;
	}
	ON_LOG(queue->tg, "%s: %s", __func__, 
			TL_NAME_FROM_ID(tl->_id));

	switch (tl->_id) {
		case id_gzip_packed:
			{
				// handle gzip
				tl_gzip_packed_t *obj =
					(tl_gzip_packed_t *)tl;

				buf_t buf;
				int _e = gunzip_buf(&buf, obj->packed_data_);
				if (_e)
				{
					char *err = gunzip_buf_err(_e);
					ON_ERR(queue->tg, "%s: %s", __func__, err);
					free(err);
				}
				tl_t *ttl = tl_deserialize(&buf);
				buf_free(buf);
				handle_tl(queue, ttl);
				if (ttl)
					tl_free(ttl);
			}
			break;
		case id_msg_container:
			{
				tl_msg_container_t *container = 
					(tl_msg_container_t *)tl; 
				ON_LOG(queue->tg, "%s: container %d long", 
						__func__, container->messages_len);
				for (i = 0; i < container->messages_len; ++i) {
					mtp_message_t m = container->messages_[i];
					// add to ack
					tg_add_msgid(queue->tg, m.msg_id);
					tl_t *ttl = tl_deserialize(&m.body);
					handle_tl(queue, ttl);
					if (ttl)
						tl_free(ttl);
				}
			}
			break;
		case id_new_session_created:
			{
				tl_new_session_created_t *obj = 
					(tl_new_session_created_t *)tl;
				// handle new session
				ON_LOG(queue->tg, "new session created...");
			}
			break;
		case id_pong:
			{
				tl_pong_t *obj = 
					(tl_pong_t *)tl;
				// handle pong
				ON_LOG(queue->tg, "pong...");
				tg_add_msgid(queue->tg, obj->msg_id_);
			}
			break;
		case id_bad_msg_notification:
			{
				tl_bad_msg_notification_t *obj = 
					(tl_bad_msg_notification_t *)tl;
				// handle bad msg notification
				tg_add_msgid(queue->tg, obj->bad_msg_id_);
				char *err = tg_strerr(tl);
				ON_ERR(queue->tg, "%s", err);
				free(err);
			}
			break;
		case id_rpc_error:
			{
				tl_rpc_error_t *rpc_error = 
					(tl_rpc_error_t *)tl;
				char *err = tg_strerr(tl);
				ON_ERR(queue->tg, "%s: %s", __func__, err);
				free(err);
			}
			break;
		case id_msgs_ack:
			{
				tl_msgs_ack_t *msgs_ack = 
					(tl_msgs_ack_t *)tl;
			}
			break;
		case id_msg_detailed_info:
			{
				tl_msg_detailed_info_t *di = 
					(tl_msg_detailed_info_t *)tl;
				tg_add_msgid(queue->tg, di->msg_id_);
				tg_queue_t *q = tg_get_queue(
						queue->tg, &di->answer_msg_id_);
				if (q){ // ok - we got msgid
					catched_tl(q, NULL);
				}
			}
			break;
		case id_msg_new_detailed_info:
			{
				tl_msg_new_detailed_info_t *di = 
					(tl_msg_new_detailed_info_t *)tl;
				tg_queue_t *q = tg_get_queue(
						queue->tg, &di->answer_msg_id_);
				if (q){ // ok - we got msgid
					catched_tl(q, NULL);
				}
			}
			break;
		case id_rpc_result:
			{
				tl_rpc_result_t *rpc_result = 
					(tl_rpc_result_t *)tl;
				// get queue in list
				tg_queue_t *q = tg_get_queue(
						queue->tg, &rpc_result->req_msg_id_);
				if (q){ // ok - we got msgid
					catched_tl(q, rpc_result->result_);
				}
			}
			break;
		case id_updatesTooLong: case id_updateShort:
		case id_updateShortMessage: case id_updateShortChatMessage:
		case id_updateShortSentMessage: case id_updatesCombined:
		case id_updates:
			{
				// do updates
				tg_do_updates(queue->tg, tl);
			}
			break;;

		default:
			break;
	}
}

static enum RTL _tg_receive(tg_queue_t *queue, int sockfd)
{
	ON_LOG(queue->tg, "%s", __func__);
	buf_t r = buf_new();
	// get length of the package
	uint32_t len;
	recv(sockfd, &len, 4, 0);
	ON_LOG(queue->tg, "%s: prepare to receive len: %d", __func__, len);
	if (len < 0) {
		// this is error - report it
		ON_ERR(queue->tg, "%s: received wrong length: %d", __func__, len);
		buf_free(r);
		return RTL_EX;
	}

	// realloc buf to be enough size
	if (buf_realloc(&r, len)){
		// handle error
		ON_ERR(queue->tg, "%s: error buf realloc to size: %d", __func__, len);
		buf_free(r);
		return RTL_EX;
	}

	// get data
	uint32_t received = 0; 
	while (received < len){
		int s = recv(
				sockfd, 
				&r.data[received], 
				len - received, 
				0);	
		if (s<0){
			ON_ERR(queue->tg, "%s: socket error: %d", __func__, s);
			buf_free(r);
			return RTL_EX;
		}
		received += s;
		
		ON_LOG(queue->tg, "%s: expected: %d, received: %d, total: %d (%d%%)", 
				__func__, len, s, received, received*100/len);

		if (queue->progress)
			if(queue->progress(queue->progressp, received, len))
				break;
	}

	// get payload 
	r.size = len;
	if (r.size == 4 && buf_get_ui32(r) == 0xfffffe6c){
		buf_free(r);
		ON_ERR(queue->tg, "%s: 404 ERROR", __func__);
		return RTL_EX;
	}

	// decrypt
	buf_t d = tg_decrypt(queue->tg, r, true);
	if (!d.size){
		buf_free(r);
		return RTL_EX;
	}
	buf_free(r);

	// deheader
	buf_t msg = tg_deheader(queue->tg, d, true);
	buf_free(d);

	// deserialize 
	tl_t *tl = tl_deserialize(&msg);
	buf_free(msg);

	// check server salt
	if (tl->_id == id_bad_server_salt){
		ON_LOG(queue->tg, "BAD SERVER SALT: resend query");
		// resend query
		return RTL_RS;
	}

	// handle tl
	handle_tl(queue, tl);
	if (tl)
		tl_free(tl);
	return RTL_RQ; // read socket again
}

static void tg_send_ack(void *data)
{
	tg_queue_t *queue = data;
	// send ACK
	if (queue->tg->msgids[0]){
		ON_LOG(queue->tg, "%s", __func__);
		buf_t ack = tg_ack(queue->tg);
		if (ack.size < 1)
			return;

		int s = 
			send(queue->socket, ack.data, ack.size, 0);
		buf_free(ack);
	}
}

static int tg_send(void *data)
{
	int err = 0;
	// send query
	tg_queue_t *queue = data;
	ON_LOG(queue->tg, "%s", __func__);
	// auth_key
	if (!queue->tg->key.size){
		err = pthread_mutex_lock(&queue->tg->queuem);
		if (err){
			ON_ERR(queue->tg, "%s: can't lock mutex: %d", __func__, err);
			return 1;
		}
		close(queue->socket);
		api.app.open();	
		queue->tg->key = 
			buf_add(shared_rc.key.data, shared_rc.key.size);
		queue->tg->salt = 
			buf_add(shared_rc.salt.data, shared_rc.salt.size);
		queue->socket = shared_rc.net.sockfd;
		queue->tg->seqn = shared_rc.seqnh + 1;
		pthread_mutex_unlock(&queue->tg->queuem);
	}

	// session id
	if (!queue->tg->ssid.size){
		err = pthread_mutex_lock(&queue->tg->queuem);
		if (err){
			ON_ERR(queue->tg, "%s: can't lock mutex: %d", __func__, err);
			return 1;
		}
		queue->tg->ssid = buf_rand(8);
		pthread_mutex_unlock(&queue->tg->queuem);
	}

	// server salt
	if (!queue->tg->salt.size){
		err = pthread_mutex_lock(&queue->tg->queuem);
		if (err){
			ON_ERR(queue->tg, "%s: can't lock mutex: %d", __func__, err);
			return 1;
		}
		queue->tg->salt = buf_rand(8);
		pthread_mutex_unlock(&queue->tg->queuem);
	}

	// prepare query
	buf_t b = tg_prepare_query(
			queue->tg, 
			queue->query, 
			true, 
			&queue->msgid);
	if (!b.size)
	{
		ON_ERR(queue->tg, "%s: can't prepare query", __func__);
		buf_free(b);
		tg_net_close(queue->tg, queue->socket);
		return 1;
	}

	// send query
	int s = 
		send(queue->socket, b.data, b.size, 0);
	if (s < 0){
		ON_ERR(queue->tg, "%s: socket error: %d", __func__, s);
		buf_free(b);
		return 1;
	}

	return 0;
}

static void * tg_run_queue(void * data)
{
	tg_queue_t *queue = data;
	ON_LOG(queue->tg, "%s", __func__);
	// open socket
	queue->socket = tg_net_open(queue->tg);
	if (queue->socket < 0)
	{
		ON_ERR(queue->tg, "%s: can't open socket", __func__);
		buf_free(queue->query);
		free(queue);
		pthread_exit(NULL);	
	}

	// add to list
	int err = 0;
	err = pthread_mutex_lock(&queue->tg->queuem);
	if (err){
		ON_ERR(queue->tg, "%s: can't lock mutex: %d", __func__, err);
		buf_free(queue->query);
		free(queue);
		pthread_exit(NULL);	
	}
	list_add(&queue->tg->queue, data);
	pthread_mutex_unlock(&queue->tg->queuem);

	// send ack
	tg_send_ack(data);
	
	// send
	if (tg_send(data))
		queue->loop = false;

	// receive loop
	while (queue->loop) {
		// receive
		ON_LOG(queue->tg, "%s: receive...", __func__);
		//usleep(1000); // in microseconds
		enum RTL res = 
			_tg_receive(queue, queue->socket);
		
		if (res == RTL_RS)
		{	
			if (tg_send(data))
				break;
		}

		if (res == RTL_EX)
			break;
	}
	if (queue->socket >= 0)
		tg_net_close(queue->tg, queue->socket);

	buf_free(queue->query);
	free(queue);
	pthread_exit(NULL);	
}

tg_queue_t * tg_queue_new(
		tg_t *tg, buf_t *query, 
		void *userdata, void (*on_done)(void *userdata, const tl_t *tl),
		void *progressp, 
		int (*progress)(void *progressp, int size, int total))
{
	ON_LOG(tg, "%s", __func__);
	tg_queue_t *queue = NEW(tg_queue_t, 
			ON_ERR(tg, "%s: can't allocate memory", __func__);
			return NULL;);

	queue->tg = tg;
	queue->loop = true;
	queue->query = buf_add_buf(*query);
	queue->userdata = userdata;
	queue->on_done = on_done;
	queue->progressp = progressp;
	queue->progress = progress;

	// start thread
	if (pthread_create(
			&(queue->p), 
			NULL, 
			tg_run_queue, 
			queue))
	{
		ON_ERR(tg, "%s: can't create thread", __func__);
		return NULL;
	}

	return queue;
}

pthread_t tg_send_query_async(tg_t *tg, buf_t *query,
		void *userdata, void (*callback)(void *userdata, const tl_t *tl))
{
	ON_LOG(tg, "%s: tg: %p, query: %p, userdata: %p, callback: %p",
		 	__func__, tg, query, userdata, callback);
	tg_queue_t *queue = 
		tg_queue_new(tg, query, userdata, callback,
			 	NULL, NULL);
	return queue->p;
}

pthread_t tg_send_query_async_with_progress(tg_t *tg, buf_t *query,
		void *userdata, void (*callback)(void *userdata, const tl_t *tl),
		void *progressp, 
		int (*progress)(void *progressp, int size, int total))
{
	ON_LOG(tg, "%s: tg: %p, query: %p, userdata: %p, callback: %p"
			       "progressp: %p, progress: %p",
		 	__func__, tg, query, userdata, callback, progressp, progress);
	tg_queue_t *queue = 
		tg_queue_new(tg, query, userdata, callback,
			 	progressp, progress);
	return queue->p;
}

static void tg_send_query_sync_cb(void *d, const tl_t *tl)
{
	tl_t **tlp = d;
	*tlp = tl_deserialize((buf_t *)(&tl->_buf));
}

tl_t *tg_send_query_sync(tg_t *tg, buf_t *query)
{
	tl_t *tl = NULL;
	tg_queue_t *queue = 
		tg_queue_new(tg, query, 
				&tl, 
				tg_send_query_sync_cb, 
				NULL, NULL);
	if (queue){
		pthread_join(queue->p, NULL);
	}

	return tl;
}

void tg_queue_cancell_all(tg_t *tg)
{
	ON_LOG(tg, "%s", __func__);
	int err = pthread_mutex_lock(&tg->queuem);
	if (err){
		ON_ERR(tg, "%s: can't lock mutex: %d", __func__, err);
		return;
	}
	tg_queue_t *queue;
	list_for_each(tg->queue, queue)
	{
		queue->loop = false;
		close(queue->socket);
	}

	list_free(&tg->queue);
	pthread_mutex_unlock(&tg->queuem);
}
