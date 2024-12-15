#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define _POSIX_C_SOURCE 199309L

#include <time.h>
#include <assert.h>
#include <stdbool.h>

#include "../tg/tg.h"
#include "../tl/alloc.h"
#include "../crypto/hsh.h"
#include "../crypto/cry.h"
#include "../transport/net.h"
//#include "../transport/crc.h"
#include "../mtx/include/api.h"
#include "transport.h"

tl_t * tg_handle_deserialized_message(tg_t *tg, tl_t *tl)
{
	int i;
	switch (tl->_id) {
		case id_msg_container:
			{
				tl_msg_container_t *obj = 
					(tl_msg_container_t *)tl;
				for (i = 0; i < obj->messages_len; ++i) {
					mtp_message_t m = obj->messages_[i];
					tl = tg_handle_serialized_message(tg, m.body);	
				}
			}
			break;
		case id_new_session_created:
			{
				tl_new_session_created_t *obj = 
					(tl_new_session_created_t *)tl;
				// handle new session
			}
			break;
		case id_pong:
			{
				tl_pong_t *obj = 
					(tl_pong_t *)tl;
				// handle pong
			}
			break;
		case id_msgs_ack:
			{
				tl_msgs_ack_t *obj = 
					(tl_msgs_ack_t *)tl;
			}
			break;
		case id_rpc_result:
			{
				tl_rpc_result_t *obj = 
					(tl_rpc_result_t *)tl;
				tl_t *result = 
					tg_handle_deserialized_message(tg, obj->result_);
				if (result){
					tl = result;
					// acknowlege
					/* TODO:  <12-11-24, kuzmich> */
				}
				
				if (result && result->_id == id_gzip_packed){
					// handle gzip
					tl_gzip_packed_t *obj =
						(tl_gzip_packed_t *)result;

					buf_t buf;
					int _e = gunzip_buf(&buf, obj->packed_data_);
					if (_e)
					{
						char *err = gunzip_buf_err(_e);
						ON_ERR(tg, "%s: %s", __func__, err);
						free(err);
					}
					tl = tg_handle_serialized_message(tg, buf);
				}
			}
			break;
		case id_bad_msg_notification:
			{
				tl_bad_msg_notification_t *obj = 
					(tl_bad_msg_notification_t *)tl;
				// handle bad msg notification
				char *err = tg_strerr(tl);
				ON_ERR(tg, "%s", err);
				free(err);
				return tl;
			}
			break;

		default:
			break;
	}

	return tl;
}


buf_t tg_prepare_query(tg_t *tg, buf_t query, bool enc)
{
	buf_t h = tg_header(tg, query, enc);
	
	buf_t e = tg_encrypt(tg, h, enc);
	buf_free(h);

	buf_t t = tg_transport(tg, e);
	buf_free(e);

	return t;
}

tl_t * tg_handle_serialized_message(tg_t *tg, buf_t msg)
{
	if (!msg.size)
		return NULL;

	tl_t *tl = tl_deserialize(&msg);
	if (!tl)
		return NULL;

	return tg_handle_deserialized_message(tg, tl);
}

buf_t tg_get_payload(tg_t *tg, buf_t buf, bool enc){
	buf_t msg = buf_new();

	buf_t tr = tg_detransport(tg, buf);
	if (!tr.size)
		return msg;
	buf_free(buf);
	
	buf_t d = tg_decrypt(tg, tr, enc);
	if (!d.size)
		return msg;
	buf_free(tr);

	msg = tg_deheader(tg, d, enc);
	buf_free(d);

	return msg;
}


tl_t * tg_send_query_to_net(
		tg_t *tg, buf_t query, bool enc, int sockfd)
{
	ON_LOG_BUF(tg, query, "%s: %s: soc: %d ", 
			__func__, enc?"API":"RFC", sockfd);

	if (!tg->salt.size)
		tg->salt = buf_rand(8);
	
	if (!tg->ssid.size){
		tg->ssid = buf_rand(8);
		ON_LOG(tg, "%s: new session...", __func__);
	}
		
	buf_t t = tg_prepare_query(tg, query, enc);	
	tg_net_send(tg, tg->sockfd, t);
	buf_free(t);
	
	buf_t r = tg_net_receive(tg, tg->sockfd);

	buf_t msg = tg_get_payload(tg, r, enc);

	if (buf_get_ui32(msg) == 0xfffffe6c){
		tl_t *tl = NEW(tl_t, return NULL);
		tl->_id = 0xfffffe6c;
		return tl;
	}

	tl_t *tl = tg_handle_serialized_message(tg, msg);

	if (tl && tl->_id == id_bad_server_salt) // resend message
		tl = tg_send_query_(tg, query, enc);
	
	if (tl && tl->_id == id_msgs_ack) // get message again
	{
		buf_free(msg);
		buf_t buf = tg_net_receive(tg, tg->sockfd);
		msg = tg_get_payload(tg, buf, enc);
		return tg_handle_serialized_message(tg, msg);
	}

	buf_free(msg);
	return tl;
}

tl_t * tg_send_query_(tg_t *tg, buf_t query, bool enc)
{
	if (!tg->net){
		tg->sockfd = tg_net_open_port(tg, 80);
		if (tg->sockfd < 0)
			return NULL;
		tg->net = true;
	}
	
	tl_t *tl = tg_send_query_to_net(tg, query, enc, tg->sockfd);
	
	// close socket
	tg_net_close(tg, tg->sockfd);
	tg->net = false;
	
	return tl;
}

tl_t * tg_send_query(tg_t *tg, buf_t s)
{
	return tg_send_query_(tg, s, true);
}

struct sq2 {
	tg_t *tg;
	buf_t query;
	void *userdata;
	int (*callback)(void *userdata, const tl_t *tl);
	void *chunkp; 
	buf_t (*chunk)(void *chunkp, uint32_t received, uint32_t total);
};

int tg_send_query2_on_done(void *p, const buf_t r){
	struct sq2 *s = p;
	
	if (r.size < 1){
		ON_ERR(s->tg, "%s: received nothing", __func__);
		return 0;
	}
	if (buf_get_ui32(r) == 0xfffffe6c){
		tl_t *tl = NEW(tl_t, return 0);
		tl->_id = 0xfffffe6c;
		if (s->callback)
			s->callback(s->userdata, tl);
		// free tl
		return 0;
	}
	
	buf_t d = tg_decrypt(s->tg, r, true);
	if (!d.size)
		return 0;
	//buf_free(r);

	buf_t msg = tg_deheader(s->tg, d, true);
	if (!msg.size)
		return 0;
	//buf_free(d);

	tl_t *tl = tg_handle_serialized_message(s->tg, msg);
	if (tl && tl->_id == id_bad_server_salt) {
		// resend message
		tg_queue_manager_send_query(s->tg, s->query,
			 	s->userdata, s->callback,
			 	s->chunkp, s->chunk);
		return 0;
	}
	
	if (tl && tl->_id == id_msgs_ack) {
		// get one more message
		return 1;
	}

	if (tl){
		ON_LOG(s->tg, "%s: received tl_%s_t (%.8x)", 
				__func__, TL_NAME_FROM_ID(tl->_id), tl->_id);
	} else {
		ON_ERR(s->tg, "%s: can't deserialize message", __func__); 
	}

	if (tl && tl->_id == id_vector) {
		tl_vector_t *vector = (tl_vector_t *)tl;
		int i;
		for (i = 0; i < vector->len_; ++i) {
			tl_t *tl = tl_deserialize(&vector->data_);
			ON_LOG(s->tg, "%s: vector item: tl_%s_t (%.8x)", 
				__func__, TL_NAME_FROM_ID(tl->_id), tl->_id);
			if (s->callback)
				if (s->callback(s->userdata, tl))
					break;	
			// free tl
		}
	}
	
	if (s->callback)
		s->callback(s->userdata, tl);
	// free tl

	return 0;
}

void tg_queue_manager_send_query(tg_t *tg, buf_t query,
		void *userdata,
		int (*callback)(void *userdata, const tl_t *tl),
		void *chunkp, 
		buf_t (*chunk)(void *chunkp, uint32_t received, uint32_t total))
{

	if (!tg->salt.size)
		tg->salt = buf_rand(8);
	
	if (!tg->ssid.size){
		tg->ssid = buf_rand(8);
		ON_LOG(tg, "%s: new session...", __func__);
	}

	buf_t msg = tg_prepare_query(tg, query, true);	
	struct sq2 *s = NEW(struct sq2, return);
	s->tg = tg;
	s->query = query;
	s->userdata = userdata;
	s->callback = callback;
	s->chunkp = chunkp;
	s->chunk = chunk;

	tg_net_add_query(tg, msg, 0, 
			s, tg_send_query2_on_done, 
			chunkp, chunk);
}
