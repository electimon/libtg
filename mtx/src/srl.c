//
//  srl.c
//  mtx
//
//  Created by Pavel Morozkin on 17.01.14.
//  Copyright (c) 2014 Pavel Morozkin. All rights reserved.
//

#include "../include/api.h"
#include "../include/buf.h"

srl_t srl_init()
{
  srl_t srl;
  return srl;
}

srl_t srl_auth()
{
  method_req_pq_t m = api.tml->methods->req_pq.init();
	printf("req_pq init done\n");
  m = api.tml->methods->req_pq.drive(m);
	printf("req_pq drive done\n");
  method_req_DH_params_t m1 = api.tml->methods->req_DH_params.init(m);
	printf("req_DH_params init done\n");
  m1 = api.tml->methods->req_DH_params.drive(m1);
	printf("req_DH_params drive done\n");
  method_set_client_DH_params_t m2 =
      api.tml->methods->set_client_DH_params.init(m, m1);
  m2 = api.tml->methods->set_client_DH_params.drive(m2);
  buf_t g_a = m2.ctor_Server_DH_inner_data.g_a.value;
  buf_t b = m2.ctor_Client_DH_Inner_Data.b;
  buf_t dh_prime = m2.ctor_Server_DH_inner_data.dh_prime.value;
  buf_t key = api.cmn.pow_mod(g_a, b, dh_prime);
  shared_rc.key = key;
  shared_rc.salt = m2.salt;
  srl_t s = {};

  return s;
}

buf_t srl_ping()
{
  buf_t r = {};
	buf_init(&r);
  method_ping_t m = api.tml->methods->ping.init();
  api.tml->methods->ping.drive(m);

  return r;
}

ctor_auth_SentCode_t srl_sendCode(const char *phone_number)
{
  method_auth_sendCode_t m = 
		api.tml->methods->auth_sendCode.init(phone_number);
	return api.tml->methods->auth_sendCode.drive(m);
}

ctor_auth_SentCode_t srl_resendCode(const char *phone_code_hash)
{
  method_auth_resendCode_t m = 
		api.tml->methods->auth_resendCode.init(phone_code_hash);
	return api.tml->methods->auth_resendCode.drive(m);
}

ctor_auth_SentCode_t srl_singIn(
		const char *phone_code_hash, const char *phone_code)
{
  method_auth_singIn_t m = 
		api.tml->methods->auth_singIn.init(phone_code_hash, phone_code);
	return api.tml->methods->auth_singIn.drive(m);
}

buf_t srl_msgsAck(ui64_t msg_id)
{
  buf_t r = buf_add_ui64(msg_id);
  method_msgs_ack_t m = 
		api.tml->methods->msgs_ack.init(r);
	api.tml->methods->msgs_ack.drive(m);

  return r;
}

abstract_t srl_initConnection(buf_t query, 
		void *userdata, method_callback_t *cb)
{
  method_initConnection_t m = 
		api.tml->methods->initConnection.init(query);
	return api.tml->methods->initConnection.drive(m);
}

abstract_t srl_invokeWithLayer(int layer, buf_t query,
		void *userdata, method_callback_t *cb)
{
  buf_t r = {};
	buf_init(&r);
  method_invokeWithLayer_t m = 
		api.tml->methods->invokeWithLayer.init(layer, query);
	return api.tml->methods->invokeWithLayer.drive(m);
}



