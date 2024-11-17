//
//  sil.c
//  mtx
//
//  Created by Pavel Morozkin on 19.01.14.
//  Copyright (c) 2014 Pavel Morozkin. All rights reserved.
//

#include "../include/api.h"
#include "../include/buf.h"
#include "../include/net.h"
#include "../include/trl.h"

typedef struct abstract_type_
{
  ui32_t id;
  param_t params[max_abstract_params];
} abstract_type_t;

// Reserved
typedef struct abstract_types_
{
  abstract_type_t type_set[max_abstract_params];
} abstract_types_t;

abstract_t sil_abstract(tg_api_type_system_t t)
{
  abstract_t a;
  a.params[0].type = TYPE_ID;

  // dispatcher must be optimized!
  if (t.method_req_pq.id__) {
    a.type = RFC;
    a.params[0].value = api.buf.add_ui32(t.method_req_pq.id__);
    a.params[1] = t.method_req_pq.nonce;
    a.size = 2;
  } else if (t.ctor_P_Q_inner_data.id__) {
    a.type = RFC;
    a.params[0].value = api.buf.add_ui32(t.ctor_P_Q_inner_data.id__);
    a.params[1] = t.ctor_P_Q_inner_data.pq;
    a.params[2] = t.ctor_P_Q_inner_data.p;
    a.params[3] = t.ctor_P_Q_inner_data.q;
    a.params[4] = t.ctor_P_Q_inner_data.nonce;
    a.params[5] = t.ctor_P_Q_inner_data.server_nonce;
    a.params[6] = t.ctor_P_Q_inner_data.new_nonce;
    a.size = 7;
  } else if (t.method_req_DH_params.id__) {
    a.type = RFC;
    a.params[0].value = api.buf.add_ui32(t.method_req_DH_params.id__);
    a.params[1] = t.method_req_DH_params.nonce;
    a.params[2] = t.method_req_DH_params.server_nonce;
    a.params[3] = t.method_req_DH_params.p;
    a.params[4] = t.method_req_DH_params.q;
    a.params[5] = t.method_req_DH_params.public_key_fingerprint;
    a.params[6] = t.method_req_DH_params.encrypted_data;
    a.size = 7;
  } else if (t.ctor_Client_DH_Inner_Data.id__) {
    a.type = RFC;
    a.params[0].value = api.buf.add_ui32(t.ctor_Client_DH_Inner_Data.id__);
    a.params[1] = t.ctor_Client_DH_Inner_Data.nonce;
    a.params[2] = t.ctor_Client_DH_Inner_Data.server_nonce;
    a.params[3] = t.ctor_Client_DH_Inner_Data.retry_id;
    a.params[4] = t.ctor_Client_DH_Inner_Data.g_b;
    a.size = 5;
  } else if (t.method_set_client_DH_params.id__) {
    a.type = RFC;
    a.params[0].value = api.buf.add_ui32(t.method_set_client_DH_params.id__);
    a.params[1] = t.method_set_client_DH_params.nonce;
    a.params[2] = t.method_set_client_DH_params.server_nonce;
    a.params[3] = t.method_set_client_DH_params.encrypted_data;
    a.size = 4;
  } else if (t.method_ping.id__) {
    a.type = API;
    a.params[0].value = api.buf.add_ui32(t.method_ping.id__);
    a.params[1] = t.method_ping.ping_id;
    a.size = 2;
  } else if (t.CodeSettings.id__) {
    a.type = RFC;
    a.params[0].value = api.buf.add_ui32(t.CodeSettings.id__);
    a.params[1] = t.CodeSettings.flags;
    //a.params[2] = t.CodeSettings.allow_flashcall;
    //a.params[3] = t.CodeSettings.current_number;
    //a.params[4] = t.CodeSettings.allow_app_hash;
    //a.params[5] = t.CodeSettings.allow_missed_call;
    //a.params[6] = t.CodeSettings.allow_firebase;
    //a.params[7] = t.CodeSettings.unknown_number;
    //a.params[8] = t.CodeSettings.logout_tokens;
    //a.params[9] = t.CodeSettings.device_token;
    //a.params[10] = t.CodeSettings.is_app_sandbox;
    a.size = 2;
  } else if (t.method_auth_sendCode.id__) {
    a.type = API;
    a.params[0].value = api.buf.add_ui32(t.method_auth_sendCode.id__);
    a.params[1] = t.method_auth_sendCode.phone_number;
		/*a.params[2] = t.method_auth_sendCode.sms_type;*/
		a.params[2] = t.method_auth_sendCode.api_id;
		a.params[3] = t.method_auth_sendCode.api_hash;
		/*a.params[5] = t.method_auth_sendCode.lang_code;*/
		a.params[4] = t.method_auth_sendCode.settings;
    a.size = 5;
	} else if (t.method_auth_resendCode.id__) {
    a.type = API;
    a.params[0].value = api.buf.add_ui32(t.method_auth_resendCode.id__);
    a.params[1] = t.method_auth_resendCode.flags;
    a.params[2] = t.method_auth_resendCode.phone_number;
		a.params[3] = t.method_auth_resendCode.phone_code_hash;
    a.size = 4;
	} else if (t.method_auth_singIn.id__) {
    a.type = API;
    a.params[0].value = api.buf.add_ui32(t.method_auth_singIn.id__);
    a.params[1] = t.method_auth_singIn.flags;
    a.params[2] = t.method_auth_singIn.phone_number;
		a.params[3] = t.method_auth_singIn.phone_code_hash;
		a.params[4] = t.method_auth_singIn.phone_code;
    a.size = 5;
  } else if (t.method_msgs_ack.id__) {
    a.type = API;
    a.params[0].value = api.buf.add_ui32(t.method_msgs_ack.id__);
    a.params[1] = t.method_msgs_ack.msg_ids;
    a.size = 2;
	} else if (t.method_initConnection.id__) {
    a.type = API;
    a.params[0].value = api.buf.add_ui32(t.method_initConnection.id__);
    a.params[1] = t.method_initConnection.flags;
    a.params[2] = t.method_initConnection.api_id;
    a.params[3] = t.method_initConnection.device_model;
    a.params[4] = t.method_initConnection.system_version;
    a.params[5] = t.method_initConnection.app_version;
    a.params[6] = t.method_initConnection.system_lang_code;
    a.params[7] = t.method_initConnection.lang_pack;
    a.params[8] = t.method_initConnection.lang_code;
    /*a.params[9] = t.method_initConnection.proxy;*/
    /*a.params[10] = t.method_initConnection.params;*/
		a.params[9] = t.method_initConnection.query;
    a.size = 10;
	} else if (t.method_invokeWithLayer.id__) {
    a.type = API;
    a.params[0].value = api.buf.add_ui32(t.method_invokeWithLayer.id__);
    a.params[1] = t.method_invokeWithLayer.layer;
    a.params[2] = t.method_invokeWithLayer.query;
    a.size = 3;
  } else {
    api.log.error("can't abstract");
  }

  return a;
}

tg_api_type_system_t t;
ui32_t reset_tg_api_type_system_flag = 0;

void reset_tg_api_type_system()
{
  tg_api_type_system_t t_ = {};
  t = t_;
}

tg_api_type_system_t sil_concrete(abstract_t a)
{
#ifdef _MSC_VER
  tg_api_type_system_t t = {};
#elif defined __GNUC__
  //tg_api_type_system_t t = {};
  if (!reset_tg_api_type_system_flag) {
    reset_tg_api_type_system();
    reset_tg_api_type_system_flag++;
  }
#endif
  ui32_t id = api.buf.get_ui32(a.params[0].value);
  param_t p;
  p.id = id;
  buf_t s = a.params[1].value; // hack
  //api.buf.dump(s);
  printf("current id: %.8x\n", id);
  switch (id) {
    case _id_resPQ:
    {
      t.ctor_ResPQ.id__ = id;
      t.ctor_ResPQ.type__ = RFC;
      p.value = s;
      p.type = TYPE_INT128;
      t.ctor_ResPQ.nonce = api.sel.deserialize_param(p);
      //api.buf.dump(t.ctor_ResPQ.nonce.value);
      s = api.buf.add(s.data + 16, s.size - 16);
      p.value = s;
      p.type = TYPE_INT128;
      t.ctor_ResPQ.server_nonce = api.sel.deserialize_param(p);
      //api.buf.dump(t.ctor_ResPQ.server_nonce.value);
      s = api.buf.add(s.data + 16, s.size - 16);
      p.value = s;
      p.type = TYPE_STRING;
      t.ctor_ResPQ.pq = api.sel.deserialize_param(p);
      //api.buf.dump(t.ctor_ResPQ.pq.value);
      ui32_t l = t.ctor_ResPQ.pq.value.size + 4; // hack
      s = api.buf.add(s.data + l, s.size - l);
      p.value = s;
      p.type = TYPE_VECTOR_LONG;
      t.ctor_ResPQ.server_public_key_fingerprints = api.sel.deserialize_param(p);
      //api.buf.dump(t.ctor_ResPQ.server_public_key_fingerprints.value);

      break;
    }
    case _id_Server_DH_Params_ok:
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
      t.ctor_Server_DH_Params.ctor_Server_DH_Params_ok = c;

      break;
    }
    case _id_server_DH_inner_data:
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
      t.ctor_Server_DH_inner_data = c;

      break;
    }
    case _id_dh_gen_ok:
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
      t.ctor_Set_client_DH_params_answer.ctor_Set_client_DH_params_answer_ok = c;

      break;
    }
    case _id_msg_container:
    {
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
			buf_init(&b);

      for (int i = 0; i < 2; ++i) {
        //api.buf.dump(p.value);
        b = api.hdl.deheader(p.value, CTER);
        abstract_t a = api.sel.deserialize(b);
        t = api.sil.concrete(a);
        ui32_t l = b.size + 16; // msg_id, seqn, len
        p.value = api.buf.add(p.value.data + l, p.value.size - l);
      }

      break;
    }
    case _id_new_session_created:
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
      t.ctor_Pong = c;

      break;
    }
		case _id_msgs_ack:
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
      
			buf_t d = api.enl.decrypt(tr, a.type);
      //api.buf.dump(d);
      
			buf_t s1r = api.hdl.deheader(d, a.type);
      //api.buf.dump(s1r);
      
			abstract_t b = api.sel.deserialize(s1r);
			sil_concrete(b);

      break;
    }
		case _id_rpc_result:
		{
			ui64_t msg_id = *(ui64_t *)(s.data);
      printf("rpc_result msg_id: %.16lx, ", msg_id);
			ui32_t *object_id = s.data + 8;
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
							c.error_code.value);
					printf("rpc_error_message: %s\n", 
							c.error_message.value);
					
					break;
				}
				//case id_auth_sentCode:
				//{
					//ctor_auth_SentCode_t c;
					//c.id__ = *object_id;
					//c.type__ = RFC;
					
					//s = api.buf.add(s.data + 12, s.size - 12);
					//int *flags = (int *)(s.data);
					//printf("sentCode flags: b%.32b\n", *flags);

					//s = api.buf.add(s.data + 4, s.size - 4);
					//int *type = (int *)(s.data);
					//printf("sentCode type: %.8x\n", *type);
					//if (*type == 0x3dbb5986 ||
							//*type == 0xc000bba2 ||
							//*type == 0x5353e5a7 )
					//{
						//s = api.buf.add(s.data + 4, s.size - 4);
						//int *lenght = (int *)(s.data);
						//printf("sentCode length: %d\n", *lenght);

						//s = api.buf.add(s.data + 4, s.size - 4);
						//p.value = s;
						//p.type = TYPE_STRING;
						//c.phone_code_hash = 
							//api.sel.deserialize_param(p);
						
						//printf("sntCode hash: %s\n", 
							//c.phone_code_hash.value);
					//}

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
					//break;
				//}	
				default: break;
			}
			break;
		}
    case _id_bad_msg_notification:
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

  return t;
}
