#include "tl.h"
#include "../mtx/include/net.h"
#include "../mtx/include/sil.h"
#include "../mtx/include/sel.h"
#include <string.h>

buf_t parse_answer(buf_t a)
{
  ui32_t id = buf_get_ui32(a);
  buf_t s = buf_add(a.data + 4, a.size - 4); // hack
  //api.buf.dump(s);
	param_t p;
  printf("current id: %.8x\n", id);
  switch (id) {
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

buf_t tl_send_tl_message(buf_t s, msg_t mtype)
{
	printf("Send:\n");
	buf_dump(s);

	buf_t s1 = api.hdl.header(s, mtype);
	api.buf.dump(s1);
  buf_t e = api.enl.encrypt(s1, mtype);
	api.buf.dump(e);
  buf_t t = api.trl.transport(e);
	//api.buf.dump(t);
  buf_t nr = api.net.drive(t, SEND_RECEIVE);
	//api.buf.dump(nr);
      
	buf_t tr = api.trl.detransport(nr);
  /*api.buf.dump(tr);*/
	buf_t d = api.enl.decrypt(tr, mtype);
  /*api.buf.dump(d);*/
  buf_t s1r = api.hdl.deheader(d, mtype);
	printf("Answer:\n");
	api.buf.dump(s1r);

	buf_t a = parse_answer(s1r);
	printf("Answer:\n");
	buf_dump(a);
	
	// acknowledge message
	//api.srl.msgsAck(*a.data + 4);
	
	return a;
}

buf_t tl_send(buf_t s)
{
	api.srl.ping();
	return tl_send_tl_message(s, API);
}
