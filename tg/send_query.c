/**
 * File              : send_query.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 03.02.2025
 * Last Modified Date: 04.02.2025
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include <pthread.h>
#include <sys/select.h>
#if INTPTR_MAX == INT32_MAX
    #define THIS_IS_32_BIT_ENVIRONMENT
		#define _LD_ "%lld"
#elif INTPTR_MAX == INT64_MAX
    #define THIS_IS_64_BIT_ENVIRONMENT
		#define _LD_ "%ld"
#else
    #error "Environment not 32 or 64-bit."
#endif


#include "tg.h"
#include "net.h"
#include "transport.h"
#include "updates.h"
#include "../mtx/include/api.h"
#include <assert.h>
#include <stdint.h>
#include <sys/socket.h>

// return msg_id or 0 on error
static uint64_t tg_send(tg_t *tg, buf_t *query, int *socket)
{
	assert(tg && query);
	int err = 0;
	
	// send query
	ON_LOG(tg, "%s: query: %s, socket: %d", 
			__func__, TL_NAME_FROM_ID(buf_get_ui32(*query)), *socket);

	// auth_key
	if (!tg->key.size){
		ON_LOG(tg, "new auth key");
		tg_net_close(tg, *socket);
		api.app.open(tg->ip, tg->port);	
		tg->key = 
			buf_add(shared_rc.key.data, shared_rc.key.size);
		tg->salt = 
			buf_add(shared_rc.salt.data, shared_rc.salt.size);
		*socket = shared_rc.net.sockfd;
		tg->seqn = shared_rc.seqnh + 1;
	}

	// session id
	if (!tg->ssid.size){
		ON_LOG(tg, "new session id");
		tg->ssid = buf_rand(8);
	}

	// server salt
	if (!tg->salt.size){
		ON_LOG(tg, "new server salt");
		tg->salt = buf_rand(8);
	}

	// prepare query
	uint64_t msg_id = 0;
	buf_t b = tg_prepare_query(
			tg, 
			query, 
			true, 
			&msg_id);
	if (!b.size)
	{
		ON_ERR(tg, "%s: can't prepare query", __func__);
		buf_free(b);
		tg_net_close(tg, *socket);
		return 0;
	}

	// send query
	int s = 
		send(*socket, b.data, b.size, 0);
	if (s < 0){
		ON_ERR(tg, "%s: socket error: %d", __func__, s);
		buf_free(b);
		return 0;
	}

	return msg_id;
}

static int tg_receive(tg_t *tg, int *sockfd, fd_set *fdset, buf_t *msg,
		void *progressp, 
		void (*progress)(void *progressp, int size, int total))
{
	assert(tg);
	ON_LOG(tg, "%s: socket: %d", __func__, *sockfd);
	
	// get length of the package
	uint32_t len;
	if (!FD_ISSET(*sockfd, fdset)){
		ON_ERR(tg, "%s: socket is closed!", __func__);
		return 1;
	}
	int s = recv(*sockfd, &len, 4, 0);
	if (s<0){
		ON_ERR(tg, "%s: %d: socket error: %d"
				, __func__, __LINE__, s);
		return 1;
	}

	ON_LOG(tg, "%s: prepare to receive len: %d", __func__, len);
	if (len < 0) {
		// this is error - report it
		ON_ERR(tg, "%s: received wrong length: %d", __func__, len);
		return 1;
	}

	// realloc buf to be enough size
	buf_t buf = buf_new();
	if (buf_realloc(&buf, len)){
		// handle error
		ON_ERR(tg, "%s: error buf realloc to size: %d", __func__, len);
		return 1;
	}

	// get data
	uint32_t received = 0; 
	while (received < len){
		int s = recv(
				*sockfd, 
				&buf.data[received], 
				len - received, 
				0);	
		if (s<0){
			ON_ERR(tg, "%s: %d: socket error: %d"
					, __func__, __LINE__, s);
			buf_free(buf);
			return 1;
		}
		received += s;
		
		ON_LOG(tg, "%s: expected: %d, received: %d, total: %d (%d%%)", 
				__func__, len, s, received, received*100/len);

		if (progress)
			progress(progressp, received, len);
	}

	// get payload 
	buf.size = len;
	if (buf.size == 4 && buf_get_ui32(buf) == 0xfffffe6c){
		ON_ERR(tg, "%s: 404 ERROR", __func__);
		buf_free(buf);
		return 1;
	}

	// decrypt
	buf_t d = tg_decrypt(tg, buf, true);
	buf_free(buf);
	if (!d.size){
		return 1;
	}

	// deheader
	*msg = tg_deheader(tg, d, true);
	buf_free(d);

	return 0;
}

static void rpc_result_from_container(
		tg_t *tg, tl_t **tl, uint64_t msg_id,
		buf_t *query, const char *ip, int port, 
		void *progressp, 
		void(*progress)(void*,int,int))
{
	assert(tl);
	if (tl[0]->_id == id_msg_container) 
	{
		tl_msg_container_t *container = 
			(tl_msg_container_t *)tl[0]; 
		//ON_LOG_BUF(tg, container->_buf, "CONTAINER: ");
		ON_LOG(tg, "%s: container %d long", 
				__func__, container->messages_len);
		int i;
		for (i = 0; i < container->messages_len; ++i) {
			mtp_message_t m = container->messages_[i];
			
			tl_t *ttl = tl_deserialize(&m.body);
			switch (ttl->_id) {
				case id_bad_msg_notification:
					{
						char *err = tg_strerr(ttl);
						ON_ERR(tg, "%s", err);
						free(err);
						tl_free(ttl);
					}
					break;

				case id_gzip_packed:
					{
						// handle gzip
						tl_gzip_packed_t *obj =
							(tl_gzip_packed_t *)ttl;

						buf_t buf;
						int _e = gunzip_buf(&buf, obj->packed_data_);
						if (_e)
						{
							char *err = gunzip_buf_err(_e);
							ON_ERR(tg, "%s: %s", __func__, err);
							free(err);
						} else {
							tl_t *tttl = tl_deserialize(&buf);
							buf_free(buf);
							if (tttl->_id == id_rpc_result){
								tl_rpc_result_t *result =
									(tl_rpc_result_t *)tttl;
								if (result->req_msg_id_ == msg_id){
									// ok - we got result
									tl_free(*tl);
									*tl = tttl;
								} else {
									ON_ERR(tg, "got rpc_result with wrong msgid");
								}
							} else {
								ON_LOG(tg, "got gzip with: %s", 
										TL_NAME_FROM_ID(tttl->_id));
								// do updates
								tg_do_updates(tg, tttl);
							}
						}
					}
					break;

				case id_updatesTooLong: case id_updateShort:
				case id_updateShortMessage: case id_updateShortChatMessage:
				case id_updateShortSentMessage: case id_updatesCombined:
				case id_updates:
					// add to ack
					tg_add_msgid(tg, m.msg_id);
					// do updates
					tg_do_updates(tg, ttl);
					tl_free(ttl);
					break;
		
				case id_msg_detailed_info:
				case id_msg_new_detailed_info:
					// add to ack
					tg_add_msgid(tg, m.msg_id);
					tl_free(ttl);
					break;
				case id_rpc_error:
					{
						tl_rpc_error_t *rpc_error = 
							(tl_rpc_error_t *)ttl;
						
						char *err = tg_strerr(ttl);
						ON_ERR(tg, "%s: %s", __func__, err);
						free(err);
						tl_free(ttl);
					}
					break; // run on_done
			
				case id_rpc_result:
					{
						// add to ack
						tg_add_msgid(tg, m.msg_id);
						// handle result
						tl_rpc_result_t *rpc_result = 
							(tl_rpc_result_t *)ttl;
						ON_LOG(tg, "got msg result: (%s) for msg_id: "_LD_"",
							rpc_result->result_?TL_NAME_FROM_ID(rpc_result->result_->_id):"NULL", 
							rpc_result->req_msg_id_);
						if (msg_id == rpc_result->req_msg_id_){
							// got result!
							ON_LOG(tg, "OK! We have result!");
							// update tl
							tl_free(*tl);
							*tl = ttl;
						} else {
							ON_ERR(tg, "rpc_result: (%s) for wrong msg_id",
								rpc_result->result_?TL_NAME_FROM_ID(rpc_result->result_->_id):"NULL"); 
							// drop!
							tg_add_todrop(tg, rpc_result->req_msg_id_);
							tl_free(ttl);
						}
					}
					break;
				case id_new_session_created:
					{
						tl_new_session_created_t *obj = 
							(tl_new_session_created_t *)ttl;
						// handle new session
						ON_LOG(tg, "new session created...");
						tl_free(ttl);
					}
					break;
				case id_msgs_ack:
					{
						ON_LOG(tg, "ACK in container");
						tl_free(*tl);
						*tl = ttl;	
					}
					break;

				default:
					ON_LOG(tg, "don't know how to handle: %s", 
							TL_NAME_FROM_ID(ttl->_id));
					tl_free(ttl);
					break;
			}
		}
	}
}

tl_t *tg_send_query_via_with_progress(tg_t *tg, buf_t *query,
		const char *ip, int port,
		void *progressp, 
		void (*progress)(void *progressp, int size, int total))
{
	assert(tg && query && ip);
	ON_LOG(tg, "%s: %s: %d", __func__, ip, port);

	fd_set fdset;
	FD_ZERO(&fdset);
	
	// open socket
	int socket = tg_net_open(tg, ip, port);
	if (socket < 0)
	{
		ON_ERR(tg, "%s: can't open socket", __func__);
		return NULL;
	}
	FD_SET(socket, &fdset);

	// lock mutex
	ON_LOG(tg, "%s: try to lock mutex...", __func__);
	if (pthread_mutex_lock(&tg->send_query))
	{
		ON_ERR(tg, "%s: can't lock mutex", __func__);
		return NULL;
	}
	ON_LOG(tg, "%s: %s: %d: catched mutex!", __func__, ip, port);
	
	// send query
	uint64_t msg_id = tg_send(tg, query, &socket);
	if (msg_id == 0){
		pthread_mutex_unlock(&tg->send_query);
		return NULL;
	}

recevive_data:;
	// reseive
	buf_t r;
	if (tg_receive(tg, &socket, &fdset, &r, progressp, progress))
	{
		pthread_mutex_unlock(&tg->send_query);
		return NULL;
	}

	// deserialize 
	tl_t *tl = tl_deserialize(&r);
	buf_free(r);

	if (tl == NULL){
		pthread_mutex_unlock(&tg->send_query);
		return NULL;
	}

	// check server salt
	if (tl->_id == id_bad_server_salt){
		ON_LOG(tg, "BAD SERVER SALT: resend query");
		// resend query
		tg_net_close(tg, socket);
		pthread_mutex_unlock(&tg->send_query);
		return tg_send_query_via_with_progress(
				tg, query, ip, port, progressp, progress);
	}
				
	ON_LOG(tg, "got answer with: %s", TL_NAME_FROM_ID(tl->_id));

	// check container
	rpc_result_from_container(
			tg, &tl, msg_id, query, ip, port, progressp, progress);

	// handle rpc_result
	if (tl->_id == id_rpc_result){
		tl_rpc_result_t *result = 
			(tl_rpc_result_t *)tl;
		ON_LOG(tg, "rpc_result with: %s",
				result->result_?TL_NAME_FROM_ID(result->result_->_id):"NULL");
		tl = result->result_;
	}

	// handle GZIP
	if (tl->_id == id_gzip_packed)
	{
		// handle gzip
		tl_gzip_packed_t *obj =
			(tl_gzip_packed_t *)tl;

		tl_t *ttl = NULL;
		buf_t buf;
		int _e = gunzip_buf(&buf, obj->packed_data_);
		if (_e)
		{
			char *err = gunzip_buf_err(_e);
			ON_ERR(tg, "%s: %s", __func__, err);
			free(err);
		} else {
			ttl = tl_deserialize(&buf);
			buf_free(buf);
			tl_free(tl);
			tl = ttl;
		}
	}

	// check bad msg
	if (tl->_id == id_bad_msg_notification){
		/* TODO: update time for correct msgid <03-02-25, yourname> */
		char *err = tg_strerr(tl);
		ON_ERR(tg, "%s", err);
		free(err);
		tl_free(tl);
		pthread_mutex_unlock(&tg->send_query);
		return NULL;
	}
	
	// check ack
	if (tl->_id == id_msgs_ack){
		tl_msgs_ack_t *ack = (tl_msgs_ack_t *)tl;
		// check msg_id
		int i;
		for (i = 0; i < ack->msg_ids_len; ++i) {
			if (msg_id == ack->msg_ids_[i]){
				ON_LOG(tg, "%s: got ACK - receive data again", __func__);
				// get data again
				tl_free(tl);
				goto recevive_data;
			}
		}
		tl_free(tl);
		pthread_mutex_unlock(&tg->send_query);
		return NULL;
	}

	// check detailed info
	if (tl->_id == id_msg_detailed_info ||
	    tl->_id == id_msg_new_detailed_info)
	{
		uint64_t msg_id_;
		if (tl->_id == id_msg_detailed_info)
			msg_id = ((tl_msg_detailed_info_t *)tl)->answer_msg_id_;
		else
			msg_id = ((tl_msg_new_detailed_info_t *)tl)->answer_msg_id_;
		if (msg_id == msg_id_){
			ON_LOG(tg, "answer has been already sended!");
		} else {
			ON_ERR(tg, "answer for wrong msgid");
		}

		tl_free(tl);
		pthread_mutex_unlock(&tg->send_query);
		return NULL;
	}
	
	// check other
	switch (tl->_id) {
		case id_updatesTooLong: case id_updateShort:
		case id_updateShortMessage: case id_updateShortChatMessage:
		case id_updateShortSentMessage: case id_updatesCombined:
		case id_updates:
			// do updates
			ON_LOG(tg, "%s: got updates", __func__);
			tg_do_updates(tg, tl);
			tl_free(tl);
			// get data again
			goto recevive_data;
			break;
		case id_rpc_error:
			{
				tl_rpc_error_t *rpc_error = 
					(tl_rpc_error_t *)tl;
				
				char *err = tg_strerr(tl);
				ON_ERR(tg, "%s: %s", __func__, err);
				free(err);
			}
			break;

		default:
			break;
	}

	pthread_mutex_unlock(&tg->send_query);
	return tl;
}

tl_t *tg_send_query_via(tg_t *tg, buf_t *query,
		const char *ip, int port)
{
	return tg_send_query_via_with_progress(
			tg, query, ip, port, 
			NULL, NULL);
}

tl_t *tg_send_query_with_progress(tg_t *tg, buf_t *query,
		void *progressp, 
		void (*progress)(void *progressp, int size, int total))
{
	return tg_send_query_via_with_progress(
			tg, query, tg->ip, tg->port, 
			progressp, progress);
}

tl_t *tg_send_query(tg_t *tg, buf_t *query)
{
	return tg_send_query_via_with_progress(
			tg, query, tg->ip, tg->port, 
			NULL, NULL);
}

//tl_t *tg_send_query_sync(tg_t *tg, buf_t *query){
	//return tg_send_query(tg, query);
//}
