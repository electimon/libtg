#include "deserialize.h"
#include "id.h"
#include "libtl.h"
#include "tl.h"
#include "types.h"
#include "../mtx/include/net.h"
#include "../mtx/include/sil.h"
#include "../mtx/include/sel.h"
#include <stdio.h>
#include <string.h>
#include <zconf.h>
#include "zlib.h"
#include <stdlib.h>

tl_t * tl_handle_serialized_message(buf_t msg);

tl_t * tl_handle_deserialized_message(tl_t *tl)
{
	int i;
	switch (tl->_id) {
		case id_msg_container:
			{
				tl_msg_container_t *obj = 
					(tl_msg_container_t *)tl;
				for (i = 0; i < obj->messages_len; ++i) {
					mtp_message_t m = obj->messages_[i];
					tl = tl_handle_serialized_message(m.body);	
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
				buf_t r = net_receive();
				buf_t tr = api.trl.detransport(r);
				buf_t d = api.enl.decrypt(tr, API);
				buf_t s1r = api.hdl.deheader(d, API);
				tl = tl_handle_serialized_message(s1r);
			}
			break;
		case id_rpc_result:
			{
				tl_rpc_result_t *obj = 
					(tl_rpc_result_t *)tl;
				tl_t *result = tl_handle_deserialized_message(obj->result_);
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
					if (gunzip_buf(&buf, obj->packed_data_))
						break;

					tl = tl_handle_serialized_message(buf);
				}
			}
			break;
		case id_bad_msg_notification:
			{
				tl_bad_msg_notification_t *obj = 
					(tl_bad_msg_notification_t *)tl;
				// handle bad msg notification
			}
			break;

		default:
			break;
	}

	return tl;
}

tl_t * tl_handle_serialized_message(buf_t msg)
{
	tl_t *tl = tl_deserialize(&msg);
	if (!tl)
		return NULL;

	return tl_handle_deserialized_message(tl);
}

tl_t * tl_send_tl_message(buf_t s, msg_t mtype)
{
	shared_rc.seqnh++;
	/*printf("message:\n");*/
	/*buf_dump(s);*/

	buf_t s1 = api.hdl.header(s, mtype);
	/*printf("+header:\n");*/
	/*api.buf.dump(s1);*/
  
	buf_t e = api.enl.encrypt(s1, mtype);
	/*printf("+crypt:\n");*/
	/*api.buf.dump(e);*/
  
	buf_t t = api.trl.transport(e);
	/*printf("+transport:\n");*/
	/*api.buf.dump(t);*/
  
	buf_t nr = api.net.drive(t, SEND_RECEIVE);
	/*printf("received:\n");*/
	/*api.buf.dump(nr);*/
      
	buf_t tr = api.trl.detransport(nr);
	/*printf("-transport:\n");*/
	/*api.buf.dump(tr);*/
	
	buf_t d = api.enl.decrypt(tr, mtype);
	/*printf("-crypt:\n");*/
	/*api.buf.dump(d);*/
  
	buf_t s1r = api.hdl.deheader(d, mtype);
	/*printf("message:\n");*/
	/*api.buf.dump(s1r);*/

	tl_t *tl = tl_handle_serialized_message(s1r);
	if (tl && tl->_id == id_bad_server_salt) // resend message
		tl = tl_send_tl_message(s, mtype);

	return tl;
}

tl_t * tl_send(buf_t s)
{
	return tl_send_tl_message(s, API);
}
