#include "../libtg.h"
#include "types.h"
#include "tl_object.h"
#include "../mtx/include/api.h"
#include "../tg/alloc.h"
#include "serialize.h"
#include "deserialize.h"
#include "../mtx/include/net.h"

struct tg_ {
	app_t app;
};

tg_t *tg_new(){
	tg_t *tg = NEW(tg_t, return NULL);	
	tg->app = api.app.open();
  
	api.log.info(".. new session");
  shared_rc.ssid = api.buf.rand(8);
	api.srl.ping();

	return tg;
}

void tg_free(tg_t *tg)
{

}

buf_t parse_answer(buf_t a)
{
  ui32_t id = api.buf.get_ui32(a);
  param_t p;
  p.id = id;
  buf_t s = buf_add(a.data + 4, a.size - 4); // hack
  //api.buf.dump(s);
  printf("current id: %.8x\n", id);
  switch (id) {
    case id_resPQ:
    {
      //t.ctor_ResPQ.id__ = id;
      //t.ctor_ResPQ.type__ = RFC;
      //p.value = s;
      //p.type = TYPE_INT128;
      //t.ctor_ResPQ.nonce = api.sel.deserialize_param(p);
      ////api.buf.dump(t.ctor_ResPQ.nonce.value);
      //s = api.buf.add(s.data + 16, s.size - 16);
      //p.value = s;
      //p.type = TYPE_INT128;
      //t.ctor_ResPQ.server_nonce = api.sel.deserialize_param(p);
      ////api.buf.dump(t.ctor_ResPQ.server_nonce.value);
      //s = api.buf.add(s.data + 16, s.size - 16);
      //p.value = s;
      //p.type = TYPE_STRING;
      //t.ctor_ResPQ.pq = api.sel.deserialize_param(p);
      ////api.buf.dump(t.ctor_ResPQ.pq.value);
      //ui32_t l = t.ctor_ResPQ.pq.value.size + 4; // hack
      //s = api.buf.add(s.data + l, s.size - l);
      //p.value = s;
      //p.type = TYPE_VECTOR_LONG;
      //t.ctor_ResPQ.server_public_key_fingerprints = api.sel.deserialize_param(p);
      //api.buf.dump(t.ctor_ResPQ.server_public_key_fingerprints.value);

      break;
    }
    case id_Server_DH_Params_ok:
    {
      ctor_Server_DH_Params_ok_t c;
      c.id__ = id;
      c.type__ = RFC;
      p.value = s;
      p.type = TYPE_INT128;
      c.nonce = api.sel.deserialize_param(p);
      //api.buf.dump(c.nonce.value);
      s = api.buf.add(s.data + 16, s.size - 16);
      p.value = s;
      p.type = TYPE_INT128;
      c.server_nonce = api.sel.deserialize_param(p);
      //api.buf.dump(c.server_nonce.value);
      s = api.buf.add(s.data + 16, s.size - 16);
      p.value = s;
      p.type = TYPE_STRING;
      c.encrypted_answer = api.sel.deserialize_param(p);
      //api.buf.dump(c.encrypted_answer.value);
      //t.ctor_Server_DH_Params.ctor_Server_DH_Params_ok = c;

      break;
    }
    case id_server_DH_inner_data:
    {
      ctor_Server_DH_inner_data_t c;
      c.id__ = id;
      c.type__ = RFC;
      p.value = s;
      p.type = TYPE_INT128;
      c.nonce = api.sel.deserialize_param(p);
      //api.buf.dump(c.nonce.value);
      s = api.buf.add(s.data + 16, s.size - 16);
      p.value = s;
      p.type = TYPE_INT128;
      c.server_nonce = api.sel.deserialize_param(p);
      //api.buf.dump(c.server_nonce.value);
      s = api.buf.add(s.data + 16, s.size - 16);
      p.value = s;
      p.type = TYPE_INT;
      c.g = api.sel.deserialize_param(p);
      //api.buf.dump(c.g.value);
      s = api.buf.add(s.data + 4, s.size - 4);
      p.value = s;
      p.type = TYPE_STRING;
      c.dh_prime = api.sel.deserialize_param(p);
      ui32_t l = c.dh_prime.value.size + 4; // hack
      //api.buf.dump(c.dh_prime.value);
      s = api.buf.add(s.data + l, s.size - l);
      p.value = s;
      p.type = TYPE_STRING;
      c.g_a = api.sel.deserialize_param(p);
      l = c.g_a.value.size + 4; // hack
      //api.buf.dump(c.g_a.value);
      s = api.buf.add(s.data + l, s.size - l);
      p.value = s;
      p.type = TYPE_INT;
      c.server_time = api.sel.deserialize_param(p);
      //api.buf.dump(c.server_time.value);
      //t.ctor_Server_DH_inner_data = c;

      break;
    }
    case id_dh_gen_ok:
    {
      ctor_Set_client_DH_params_answer_ok_t c;
      c.id__ = id;
      c.type__ = RFC;
      p.value = s;
      p.type = TYPE_INT128;
      c.nonce = api.sel.deserialize_param(p);
      //api.buf.dump(c.nonce.value);
      s = api.buf.add(s.data + 16, s.size - 16);
      p.value = s;
      p.type = TYPE_INT128;
      c.server_nonce = api.sel.deserialize_param(p);
      //api.buf.dump(c.server_nonce.value);
      s = api.buf.add(s.data + 16, s.size - 16);
      p.value = s;
      p.type = TYPE_INT128;
      c.new_nonce_hash1 = api.sel.deserialize_param(p);
      //api.buf.dump(c.new_nonce_hash1.value);
      //t.ctor_Set_client_DH_params_answer.ctor_Set_client_DH_params_answer_ok = c;

      break;
    }
    case id_msg_container:
    {
      //api.buf.dump(s);
      ctor_MessageContainer_t c;
      c.id__ = id;
      c.type__  = API;
      p.value = s;
      p.type = TYPE_VECTOR_MESSAGE;
      buf_t content = api.buf.add(p.value.data, p.value.size); // hack
      p.value = api.buf.add_ui32(id_Vector); // hack
      p.value = api.buf.cat(p.value, content);
      p = api.sel.deserialize_param(p);
      buf_t b;

      for (int i = 0; i < 2; ++i) {
        //api.buf.dump(p.value);
        b = api.hdl.deheader(p.value, CTER);
        abstract_t a = api.sel.deserialize(b);
        //t = api.sil.concrete(a);
        ui32_t l = b.size + 16; // msg_id, seqn, len
        p.value = api.buf.add(p.value.data + l, p.value.size - l);
      }

      break;
    }
    case id_new_session_created:
    {
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
      //t.ctor_NewSession = c;

      break;
    }
    case id_pong:
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
		case id_msgs_ack:
    {
/* A server usually acknowledges the receipt of a message
 * from a client (normally, an RPC query) using an RPC
 * response. If a response is a long time coming, a
 * server may first send a receipt acknowledgment, and
 * somewhat later, the RPC response itself. */
      ctor_MsgsAck_t c;
      c.id__ = id;
      c.type__ = RFC;
      p.value = s;
      p.type = TYPE_VECTOR_LONG;
			c.msg_ids = 
				api.sel.deserialize_param(p);
      //s = api.buf.add(s.data + 8, s.size - 8);
      //p.value = s;
      //p.type = TYPE_LONG;
      //c.ping_id = api.sel.deserialize_param(p);
      //api.buf.dump(c.ping_id.value);
      //t.ctor_MsgsAck = c;

			ui64_t *msg_id = s.data;
      printf("msgs_ack: %.8x\n", *msg_id);
			
			// waight for message
			buf_t r = net_receive();
      //api.buf.dump(r);

			buf_t tr = api.trl.detransport(r);
      //api.buf.dump(tr);
      
			buf_t d = api.enl.decrypt(tr, API);
      //api.buf.dump(d);
      
			buf_t s1r = api.hdl.deheader(d, API);
      //api.buf.dump(s1r);
      
			//abstract_t b = api.sel.deserialize(s1r);
			//sil_concrete(b);

      break;
    }
		case id_rpc_result:
		{
			ui64_t msg_id = *(ui64_t *)(s.data);
      printf("rpc_result msg_id: %.16lx, ", msg_id);
			ui32_t *object_id = s.data + 8;
      printf("object_id: %.8x\n", *object_id);
			
			switch (*object_id) {
				case id_rpc_error:
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
							c.error_code.value);
					printf("rpc_error_message: %s\n", 
							c.error_message.value);
					
					break;
				}
				case id_auth_sentCode:
				{
					ctor_auth_SentCode_t c;
					c.id__ = *object_id;
					c.type__ = RFC;
					
					s = api.buf.add(s.data + 12, s.size - 12);
					int *flags = (int *)(s.data);
					printf("sentCode flags: b%.32b\n", *flags);

					s = api.buf.add(s.data + 4, s.size - 4);
					int *type = (int *)(s.data);
					printf("sentCode type: %.8x\n", *type);
					if (*type == 0x3dbb5986 ||
							*type == 0xc000bba2 ||
							*type == 0x5353e5a7 )
					{
						s = api.buf.add(s.data + 4, s.size - 4);
						int *lenght = (int *)(s.data);
						printf("sentCode length: %d\n", *lenght);

						s = api.buf.add(s.data + 4, s.size - 4);
						p.value = s;
						p.type = TYPE_STRING;
						c.phone_code_hash = 
							api.sel.deserialize_param(p);
						
						printf("sntCode hash: %s\n", 
							c.phone_code_hash.value);
					}

					//t.msg_id = msg_id;	
					
					// acknolage
					/*buf_t msg = {};*/
					/*buf_add_ui64(msg_id);*/
					/*method_msgs_ack_t m = */
						/*method_msgs_ack_init(msg);*/
					/*method_msgs_ack_drive(m);*/
					
					//tg_api_type_system_t t = {};
					//t.method_msgs_ack = m;
					//abstract_t a = api.sil.abstract(t);
					//a.stk_mode = SEND_RECEIVE;
					//a = api.stk.drive(a);
					//api.sil.concrete(a).ctor_auth_SentCode;

					// handle with setnCode
					//ctor_auth_SentCode_t c;
					//c.id__ = *object_id;
					//c.type__ = API;
					//p.value = s;
					//p.type = TYPE_LONG;
					
					//api.buf.dump(c.msg_id.value);
					//s = api.buf.add(s.data + 8, s.size - 8);
					//p.value = s;
					//p.type = TYPE_LONG;
					//c.ping_id = api.sel.deserialize_param(p);
					////api.buf.dump(c.ping_id.value);
					//t.ctor_Pong = c;

					//t.ctor_auth_SentCode = c;
					break;
				}	
				default: break;
			}
			break;
		}
    case id_bad_msg_notification:
    {
      printf("current id: %.8x\n", id);
			long *id = s.data;
			int *seqn = s.data + 8;
			int *code = s.data + 12;
      api.log.info(
					"id_bad_msg_notification received");
      printf(
					"ID: %0.8x "
					"SQN: %d " 
					"CODE: %d\n", 
					 *id, *seqn, *code);

      break;
    }
    default:
    {
      printf("current id: %.8x\n", id);
      api.log.error("unknown id");

      break;
    }
  }

  //return t;
}


tlo_t * tg_send(tg_t *tg, tlo_t *object)
{
	api.srl.ping();
	buf_t s = tg_serialize(object);
  /*api.buf.dump(s);*/
  buf_t s1 = api.hdl.header(s, API);
  /*api.buf.dump(s1);*/
  buf_t e = api.enl.encrypt(s1, API);
  /*api.buf.dump(e);*/
  buf_t t = api.trl.transport(e);
  /*api.buf.dump(t);*/
  buf_t nr = api.net.drive(t, SEND_RECEIVE);
  /*api.buf.dump(nr);*/
      
	buf_t tr = api.trl.detransport(nr);
  /*api.buf.dump(tr);*/
	buf_t d = api.enl.decrypt(tr, API);
  /*api.buf.dump(d);*/
  buf_t s1r = api.hdl.deheader(d, API);
  /*api.buf.dump(s1r);*/

	parse_answer(s1r);
	
	//tlo_t *answer = tg_deserialize(&d);
	//if (answer)
		//printf("ANSWER: %.8x\n", answer->id);
	//else
		//printf("answer is NULL\n");

	return 0;
}
