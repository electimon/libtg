//
//  scl.c
//  mtx
//
//  Created by Pavel Morozkin on 24.01.14.
//  Copyright (c) 2014 Pavel Morozkin. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include "../include/api.h"
#include "../include/types.h"
#include "../include/setup.h"
#include "../include/sel.h"

extern scl_t scl_open()
{
  api.net.open(_ip, _port);
	printf("net open done\n");
  api.srl.init();
  api.log.info(">> auth");
  api.srl.auth();
  api.log.info("<< key");
  scl_t s;

  return s;
}

extern void scl_run()
{
  api.log.info(".. new session");
  shared_rc.ssid = api.buf.rand(8);
  /*ui32_t q;*/
  /*api.log.info("how many pings?");*/
  /*scanf("%d",&q);*/

  /*for (ui32_t i = 0; i < q; ++i) {*/
    /*printf(product);*/
    /*printf(": ping (try #%d)\n", i);*/
		//api.srl.ping();
  /*}*/

	/*api.srl.sendCode();*/

	//char phone_number[128];
	//api.log.info("enter phone number (+7XXXXXXXXXX)");
	//scanf("%s",phone_number);
	
	/*method_auth_sendCode_t sendCode = */
		/*method_auth_sendCode_init("9996623333");*/
	/*buf_t sendCode_buf = */
		/*SERIALIZE_METHOD(method_auth_sendCode, sendCode);*/
	
	/*printf("sendCode:\n");*/
	/*api.buf.dump(sendCode_buf);*/

	/*method_initConnection_t initConnection =*/
		/*method_initConnection_init(sendCode_buf);*/
	/*buf_t initConnection_buf = */
		/*SERIALIZE_METHOD(method_initConnection, initConnection);*/

	/*printf("initConnection:\n");*/
	/*api.buf.dump(initConnection_buf);*/

	/*return;*/
	/*abstract_t a = */
		/*api.srl.invokeWithLayer(185, initConnection_buf);*/
	/*//abstract_t a = api.srl.initConnection(sendCode_buf);*/
	/*tg_api_type_system_t t = api.sil.concrete(a);*/
	/*ctor_auth_SentCode_t c = t.ctor_auth_SentCode;*/

	/*char phone_code_hash[32];*/
	/*strcpy(phone_code_hash, c.phone_code_hash.value.data);*/
	/*printf("phone_code_hash: %s\n", phone_code_hash);*/
	
	/*[>api.srl.msgsAck(t.msg_id);<]*/
	
	/*api.srl.ping();*/
	/*//api.srl.resendCode(c.phone_code_hash.value.data);*/

	/*int code;*/
	/*api.log.info("enter code?");*/
	/*scanf("%d",&code);*/
	/*printf("code: %d\n", code);*/

	/*char phone_code[32];*/
	/*sprintf(phone_code, "%d", code);*/
	/*printf("phone_code: %s\n", phone_code);*/

	/*api.srl.singIn(phone_code_hash, phone_code);*/
	
	printf("press any key\n");
	getchar();
}

extern void scl_close()
{
  net_t n = shared_rc_get_net();
  api.net.close(n);
}
