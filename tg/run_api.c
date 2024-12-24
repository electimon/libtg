#include "tg.h"
#include "../transport/transport.h"
#include "../transport/net.h"
#include <unistd.h>
#include <sys/socket.h>

tl_t * tg_deserialize(tg_t *tg, buf_t *buf)
{
	int i;
	tl_t *tl = tl_deserialize(buf);
	if (tl == NULL){
		ON_ERR(tg, "%s: can't deserialize data", __func__);
		return NULL;
	}

	switch (tl->_id) {
		case id_msg_container:
			{
				tl_msg_container_t *obj = 
					(tl_msg_container_t *)tl;
				for (i = 0; i < obj->messages_len; ++i) {
					mtp_message_t m = obj->messages_[i];
					tl = tg_deserialize(tg, &m.body);	
				}
			}
			break;
		case id_new_session_created:
			{
				tl_new_session_created_t *obj = 
					(tl_new_session_created_t *)tl;
				// handle new session
				ON_LOG(tg, "new session created...");
			}
			break;
		case id_pong:
			{
				tl_pong_t *obj = 
					(tl_pong_t *)tl;
				// handle pong
				ON_LOG(tg, "pong...");
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
				return NULL;
			}
			break;
		case id_rpc_error:
			{
				char *err = tg_strerr(tl);
				ON_ERR(tg, "%s", err);
				free(err);
				return NULL;
			}
			break;

		default:
			break;
	}

	return tl;
}

tl_t *tg_result(tg_t *tg, tl_t *result)
{
	tl_t *tl = result;
	printf("%s: handle result with tl: %s\n", 
			__func__, TL_NAME_FROM_ID(result->_id));
	switch (result->_id) {
		case id_gzip_packed:
			{
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
				tl = tg_deserialize(tg, &buf);
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
				return NULL;
			}
			break;
		case id_rpc_error:
			{
				char *err = tg_strerr(tl);
				ON_ERR(tg, "%s", err);
				free(err);
				return NULL;
			}
			break;

		default:
			break;
	}

	return tl;
}

tl_t *tg_run_api(tg_t *tg, buf_t *query)
{
	int i;
	tl_t *tl = NULL;

	// wait to unlock
	for (i=0; i<100; ++i) {
		if (!tg->send_lock)
			break;
		usleep(1000); // in microseconds
	}
	if (i>=100)
		return tl;
	tg->send_lock = true;

	// prepare query
	uint64_t msgid = 0;
	buf_t b = tg_prepare_query(
			tg, *query, true, &msgid);
	if (!b.size)
		goto tg_run_api_end;

	// open socket
	int sockfd = tg_net_open(tg);
	if (sockfd < 0)
		goto tg_run_api_end;

	// send ACK
	if (tg->msgids[0]){
		buf_t ack = tg_ack(tg);
		int s = 
			send(sockfd, ack.data, ack.size, 0);
		buf_free(ack);
		if (s < 0){
			ON_ERR(tg, "%s: socket error: %d", __func__, s);
			goto tg_run_api_end;
		}
	}

	// send query
	ON_LOG(tg, "%s: send: %s", __func__, buf_sdump(b));
	int s = 
		send(sockfd, b.data, b.size, 0);
	if (s < 0){
		ON_ERR(tg, "%s: socket error: %d", __func__, s);
		goto tg_run_api_end;
	}

	// receive data
tg_run_api_receive_data:;
	buf_t r = buf_new();
	// get length of the package
	uint32_t len;
	recv(sockfd, &len, 4, 0);
	ON_LOG(tg, "%s: prepare to receive len: %d", __func__, len);
	if (len < 0) {
		// this is error - report it
		ON_ERR(tg, "%s: received wrong length: %d", __func__, len);
		buf_free(r);
		goto tg_run_api_end;
	}

	// realloc buf to be enough size
	if (buf_realloc(&r, len)){
		// handle error
		ON_ERR(tg, "%s: error buf realloc to size: %d", __func__, len);
		buf_free(r);
		goto tg_run_api_end;
	}

	// get data
	uint32_t received = 0; 
	while (received < len){
		int s = 
			recv(sockfd, &r.data[received], len - received, 0);	
		if (s<0){
			ON_ERR(tg, "%s: socket error: %d", __func__, s);
			buf_free(r);
			goto tg_run_api_end;
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
		buf_free(r);
		goto tg_run_api_end;
	}

	// get payload 
	r.size = len;
	if (r.size == 4 && buf_get_ui32(r) == 0xfffffe6c){
		buf_free(r);
		goto tg_run_api_end;
	}

	buf_t d = tg_decrypt(tg, r, true);
	if (!d.size){
		buf_free(r);
		goto tg_run_api_end;
	}
	buf_free(r);

	buf_t msg = tg_deheader(tg, d, true);
	if (!msg.size){
		buf_free(d);
		goto tg_run_api_end;
	}
	buf_free(d);

	// deserialize 
	tl = tg_deserialize(tg, &msg);
	buf_free(r);
	
	// handle tl
	if (tl == NULL)
		goto tg_run_api_end;

	if (tl->_id != id_rpc_result){
		ON_LOG(tg, "%s: expected rpc_result, but got: %s", 
				__func__, TL_NAME_FROM_ID(tl->_id));
		// receive data again
		goto tg_run_api_receive_data;
	} 
	
	// check msgid
	tl_rpc_result_t *result = (tl_rpc_result_t *)tl;
	if (msgid != result->req_msg_id_){
		ON_ERR(tg, "%s: rpc result with wrong msg id", __func__);
		// free tl
		tl_free(tl);
		tl = NULL;
		goto tg_run_api_end;
	}
	// handle result
	if (result->result_ == NULL){
		ON_ERR(tg, "%s: rpc result is NULL", __func__);
		// free tl
		tl_free(tl);
		tl = NULL;
		goto tg_run_api_end;
	}
	tl = tg_result(tg, result->result_);

tg_run_api_end:;
	buf_free(b);
	tg->send_lock = false;
	return tl;	
}
