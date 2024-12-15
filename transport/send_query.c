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

tl_t * tg_handle_serialized_message2(tg_t *tg, buf_t msg);

tl_t * tg_handle_deserialized_message(tg_t *tg, tl_t *tl, int sockfd)
{
	int i;
	switch (tl->_id) {
		case id_msg_container:
			{
				tl_msg_container_t *obj = 
					(tl_msg_container_t *)tl;
				for (i = 0; i < obj->messages_len; ++i) {
					mtp_message_t m = obj->messages_[i];
					tl = tg_handle_serialized_message(tg, m.body, sockfd);	
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
				// get new message
				buf_t r = tg_net_receive(tg, sockfd);
				buf_t tr = tg_detransport(tg, r);
				buf_free(r);
				if (buf_get_ui32(tr) == 0xfffffe6c){
					tl_t *tl = NEW(tl_t, return NULL);
					tl->_id = buf_get_ui32(tr);
					return tl;
				}
				buf_t d = tg_decrypt(tg, tr, true);
				buf_free(tr);
				buf_t msg = tg_deheader(tg, d, true);
				buf_free(d);
				tl = tg_handle_serialized_message(tg, msg, sockfd);
			}
			break;
		case id_rpc_result:
			{
				tl_rpc_result_t *obj = 
					(tl_rpc_result_t *)tl;
				tl_t *result = 
					tg_handle_deserialized_message(tg, obj->result_, sockfd);
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
						ON_ERR(tg, result, "%s: %s", __func__, err);
						free(err);
					}
					tl = tg_handle_serialized_message(tg, buf, sockfd);
				}
			}
			break;
		case id_bad_msg_notification:
			{
				tl_bad_msg_notification_t *obj = 
					(tl_bad_msg_notification_t *)tl;
				// handle bad msg notification
				char *err = tg_strerr(tl);
				ON_ERR(tg, tl, "%s", err);
				free(err);
				return tl;
			}
			break;

		default:
			break;
	}

	return tl;
}

tl_t * tg_handle_deserialized_message2(tg_t *tg, tl_t *tl)
{
	int i;
	switch (tl->_id) {
		case id_msg_container:
			{
				tl_msg_container_t *obj = 
					(tl_msg_container_t *)tl;
				for (i = 0; i < obj->messages_len; ++i) {
					mtp_message_t m = obj->messages_[i];
					tl = tg_handle_serialized_message2(tg, m.body);	
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
					tg_handle_deserialized_message2(tg, obj->result_);
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
						ON_ERR(tg, result, "%s: %s", __func__, err);
						free(err);
					}
					tl = tg_handle_serialized_message2(tg, buf);
				}
			}
			break;
		case id_bad_msg_notification:
			{
				tl_bad_msg_notification_t *obj = 
					(tl_bad_msg_notification_t *)tl;
				// handle bad msg notification
				char *err = tg_strerr(tl);
				ON_ERR(tg, tl, "%s", err);
				free(err);
				return tl;
			}
			break;

		default:
			break;
	}

	return tl;
}


buf_t tg_serialize_query(tg_t *tg, buf_t query, bool enc)
{
	buf_t h = tg_header(tg, query, enc);
	
	buf_t e = tg_encrypt(tg, h, enc);
	buf_free(h);

	buf_t t = tg_transport(tg, e);
	buf_free(e);

	return t;
}

tl_t * tg_handle_serialized_message(tg_t *tg, buf_t msg, int sockfd)
{
	if (!msg.size)
		return NULL;

	tl_t *tl = tl_deserialize(&msg);
	if (!tl)
		return NULL;

	return tg_handle_deserialized_message(tg, tl, sockfd);
}

tl_t * tg_handle_serialized_message2(tg_t *tg, buf_t msg)
{
	if (!msg.size)
		return NULL;

	tl_t *tl = tl_deserialize(&msg);
	if (!tl)
		return NULL;

	return tg_handle_deserialized_message2(tg, tl);
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
		
	// DRIVE
	buf_t msg;

	buf_t t = tg_serialize_query(tg, query, enc);	
	tg_net_send(tg, tg->sockfd, t);
	buf_free(t);
	
	buf_t r = tg_net_receive(tg, tg->sockfd);

	buf_t tr = tg_detransport(tg, r);
	if (!tr.size)
		return NULL;
	buf_free(r);
	if (buf_get_ui32(tr) == 0xfffffe6c){
		tl_t *tl = NEW(tl_t, return NULL);
		tl->_id = 0xfffffe6c;
		return tl;
	}
	if (tr.size == 4 && buf_get_ui32(tr) == -405){
		sleep(1);
		// send message again
		return tg_send_query_(tg, query, enc);
	}

	buf_t d = tg_decrypt(tg, tr, enc);
	if (!d.size)
		return NULL;

	buf_free(tr);

	msg = tg_deheader(tg, d, enc);
	if (!msg.size)
		return NULL;
	buf_free(d);

	tl_t *tl = tg_handle_serialized_message(tg, msg, sockfd);
	if (tl && tl->_id == id_bad_server_salt) // resend message
		tl = tg_send_query_(tg, query, enc);

	buf_free(msg);
	
	return tl;
}

tl_t * tg_send_query_(tg_t *tg, buf_t query, bool enc)
{
	if (!tg->net){
		tg->sockfd = tg_net_open(tg);
		tg->net = true;
	}
	return tg_send_query_to_net(tg, query, enc, tg->sockfd);
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
		ON_ERR(s->tg, NULL, "%s: received nothing", __func__);
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
	
	if (r.size == 4 && buf_get_ui32(r) == -405){
		sleep(1);
		// send message again
		tg_send_query2(s->tg, s->query,
			 	s->userdata, s->callback,
			 	s->chunkp, s->chunk);
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

	tl_t *tl = tg_handle_serialized_message2(s->tg, msg);
	if (tl && tl->_id == id_bad_server_salt) {
		// resend message
		tg_send_query2(s->tg, s->query,
			 	s->userdata, s->callback,
			 	s->chunkp, s->chunk);
		return 0;
	}
	
	if (tl && tl->_id == id_msgs_ack) {
		// get one more message
		return 1;
	}

	if (tl){
		ON_LOG(s->tg, "%s: received %s (%.8x)", 
				__func__, TL_NAME_FROM_ID(tl->_id), tl->_id);
	} else {
		ON_ERR(s->tg, NULL, "%s: can't deserialize message", __func__); 
	}

	if (tl && tl->_id == id_vector) {
		tl_vector_t *vector = (tl_vector_t *)tl;
		int i;
		for (i = 0; i < vector->len_; ++i) {
			tl_t *tl = tl_deserialize(&vector->data_);
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

void tg_send_query2(tg_t *tg, buf_t query,
		void *userdata,
		int (*callback)(void *userdata, const tl_t *tl),
		void *chunkp, 
		buf_t (*chunk)(void *chunkp, uint32_t received, uint32_t total))
{
	buf_t msg = tg_serialize_query(tg, query, true);	
	struct sq2 *s = NEW(struct sq2, return);
	s->tg = tg;
	s->query = query;
	s->userdata = userdata;
	s->callback = callback;
	s->chunkp = chunkp;
	s->chunk = chunk;

	tg_net_add_query(tg, msg, 0, 
			s, tg_send_query2_on_done, 
			NULL, NULL);
}
