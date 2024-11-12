#include "deserialize.h"
#include "id.h"
#include "tl.h"
#include "types.h"
#include "struct.h"
#include "../mtx/include/net.h"
#include "../mtx/include/sil.h"
#include "../mtx/include/sel.h"
#include <stdio.h>
#include <string.h>
#include <zconf.h>
#include "zlib.h"

buf_t parse_answer(buf_t a)
{
  ui32_t id = buf_get_ui32(a);
  buf_t s = buf_add(a.data + 4, a.size - 4); // hack
  //api.buf.dump(s);
	param_t p;
  printf("current id: %.8x\n", id);
  switch (id) {
		case _id_msg_container:
    {
			tg_api_type_system_t t;
      //api.buf.dump(s);
      ctor_MessageContainer_t c;
      c.id__ = id;
      c.type__  = API;
      p.value = s;
      p.type = TYPE_VECTOR_MESSAGE;
      buf_t content = api.buf.add(p.value.data, p.value.size); // hack
      p.value = api.buf.add_ui32(_id_Vector); // hack
      p.value = api.buf.cat(p.value, content);
      p = api.sel.deserialize_param(p);
      buf_t b;

      /*for (int i = 0; i < 2; ++i) {*/
        /*//api.buf.dump(p.value);*/
        /*b = api.hdl.deheader(p.value, CTER);*/
        /*abstract_t a = api.sel.deserialize(b);*/
        /*t = api.sil.concrete(a);*/
        /*ui32_t l = b.size + 16; // msg_id, seqn, len*/
        /*p.value = api.buf.add(p.value.data + l, p.value.size - l);*/
				/*printf("MSG: %s\n", p.value.data);*/
      /*}*/

      break;
    }
    case _id_new_session_created:
    {
			tg_api_type_system_t t;
      ctor_NewSession_t c;
      c.id__ = id;
      c.type__ = API;
      p.value = s;
      p.type = TYPE_LONG;
      c.first_msg_id = api.sel.deserialize_param(p);
      //api.buf.dump(c.first_msg_id.value);
      s = api.buf.add(s.data + 8, s.size - 8);
      p.value = s;
      p.type = TYPE_LONG;
      c.unique_id = api.sel.deserialize_param(p);
      //api.buf.dump(c.unique_id.value);
      s = api.buf.add(s.data + 8, s.size - 8);
      p.value = s;
      p.type = TYPE_LONG;
      c.server_salt = api.sel.deserialize_param(p);
      //api.buf.dump(c.server_salt.value);
      t.ctor_NewSession = c;

      break;
    }

		case _id_pong:
    {
      ctor_Pong_t c;
      c.id__ = id;
      c.type__ = API;
      p.value = s;
      p.type = TYPE_LONG;
      c.msg_id = api.sel.deserialize_param(p);
      //api.buf.dump(c.msg_id.value);
      s = api.buf.add(s.data + 8, s.size - 8);
      p.value = s;
      p.type = TYPE_LONG;
      c.ping_id = api.sel.deserialize_param(p);
      //api.buf.dump(c.ping_id.value);
      //t.ctor_Pong = c;
      break;
    }
		case _id_msgs_ack:
    {
/* A server usually acknowledges the receipt of a message
 * from a client (normally, an RPC query) using an RPC
 * response. If a response is a long time coming, a
 * server may first send a receipt acknowledgment, and
 * somewhat later, the RPC response itself. */
      
			ui64_t *msg_id = (ui64_t *)(s.data);
      printf("msgs_ack: %.16lx\n", *msg_id);
			
			// waight for message
			buf_t r = net_receive();
      //api.buf.dump(r);

			buf_t tr = api.trl.detransport(r);
      //api.buf.dump(tr);
      
			buf_t d = api.enl.decrypt(tr, API);
      //api.buf.dump(d);
      
			buf_t s1r = api.hdl.deheader(d, API);
      //api.buf.dump(s1r);
			
			parse_answer(s1r);

      break;
    }
		case _id_rpc_result:
		{
			ui64_t msg_id = *(ui64_t *)(s.data);
      printf("rpc_result msg_id: %.16lx, ", msg_id);
			ui32_t *object_id = (ui32_t *)(s.data + 8);
      printf("object_id: %.8x\n", *object_id);
			
			switch (*object_id) {
				case _id_rpc_error:
				{
					//RPC Error
					ctor_rpc_error_t c;
					c.id__ = *object_id;
					c.type__ = RFC;
					
					s = api.buf.add(s.data + 12, s.size - 12);
					p.value = s;
					p.type = TYPE_INT;
					c.error_code = api.sel.deserialize_param(p);
					
					s = api.buf.add(s.data + 4, s.size - 4);
					p.value = s;
					p.type = TYPE_STRING;
					c.error_message = api.sel.deserialize_param(p);

					printf("rpc_error_code: %.8x, ", 
							*(int*)(c.error_code.value.data));
					printf("rpc_error_message: %s\n", 
							c.error_message.value.data);
					
					buf_t buf = {};
					memset(&buf, 0, sizeof(buf));
					return buf;
				}
				default: break;
			}
			s = buf_add(s.data + 8, s.size - 8);
			return s;
			break;
		}
    case _id_bad_msg_notification:
    {
      printf("current id: %.8x\n", id);
			long *id = (long *)(s.data);
			int *seqn = (int *)(s.data + 8);
			int *code = (int *)(s.data + 12);
      api.log.info(
					"id_bad_msg_notification received");
      printf(
					"ID: %0.16lx "
					"SQN: %d " 
					"CODE: %d\n", 
					 *id, *seqn, *code);

      break;
    }
    default:
    {
      printf("current id: %.8x\n", id);
      break;
    }
  }
  return a;
}

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

					buf_dump(obj->packed_data_);

					if (obj->packed_data_.size > max_buf_size){
						// buffer overload
						perror("buffer overload");
						break;
					}

					buf_t buf;
					unsigned long len = max_buf_size;
					z_stream strm;
					if (inflateInit2(&strm, 16 + MAX_WBITS) != Z_OK) break;
					strm.avail_in = obj->packed_data_.size;
					strm.next_in = obj->packed_data_.data;
					strm.avail_out = len;
					strm.next_out = buf.data;
					int ret = inflate(&strm, Z_FINISH);
					if (ret != Z_OK && ret != Z_STREAM_END){
						printf("uncompress error: %d\n", ret);
						break;	
					}
					printf("total_out: %ld\n", strm.total_out);
					inflateEnd(&strm);
					buf.size = strm.total_out;
					
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
	//printf("Send:\n");
	//buf_dump(s);

	buf_t s1 = api.hdl.header(s, mtype);
	//api.buf.dump(s1);
  buf_t e = api.enl.encrypt(s1, mtype);
	//api.buf.dump(e);
  buf_t t = api.trl.transport(e);
	//api.buf.dump(t);
  buf_t nr = api.net.drive(t, SEND_RECEIVE);
	//api.buf.dump(nr);
      
	buf_t tr = api.trl.detransport(nr);
	//printf("Answer:\n");
	//api.buf.dump(tr);
	buf_t d = api.enl.decrypt(tr, mtype);
  /*api.buf.dump(d);*/
  buf_t s1r = api.hdl.deheader(d, mtype);
	//printf("Answer:\n");
	//api.buf.dump(s1r);

	tl_t *tl = tl_handle_serialized_message(s1r);
	if (tl && tl->_id == id_bad_server_salt) // resend message
		tl = tl_send_tl_message(s, mtype);

	return tl;
}

tl_t * tl_send(buf_t s)
{
	return tl_send_tl_message(s, API);
}
