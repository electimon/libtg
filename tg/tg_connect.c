#include "tg.h"
#include "strtok_foreach.h"
#include <stdio.h>
#include <string.h>
#include "../mtx/include/net.h"

int tg_connect(
		tg_t *tg,
		void *userdata,
		char * (*callback)(
			void *userdata,
			TG_AUTH auth,
			const tl_t *tl))
{
	// try to load auth_key_id from database
	buf_t auth_key = auth_key_from_database(tg);

	if (auth_key.size){
		printf("Have auth_key\n");
		reset_shared_rc();
		net_open(_ip, _port);
		shared_rc.key = buf_add(auth_key.data, auth_key.size);
		api.srl.init();
		/*api.log.info(".. new session");*/
		shared_rc.ssid = api.buf.rand(8);
		//buf_t get_salt = tl_get_future_salts(1);
		
		//buf_t nonce = buf_rand(16);
		buf_t get_salt = tl_get_future_salts(1);
		tl_send_tl_message(get_salt, RFC);
		
		// check if authorized
		//InputUser iuser = tl_inputUserSelf();
		//buf_t getUsers = tl_users_getUsers(&iuser, 1);	
		//buf_t answer = tl_send(getUsers); 
		//printf("ANSWER ID: %.8x\n", id_from_tl_buf(answer));
		return 0;
	}

	// try to get phone_number
	char * phone_number = phone_number_from_database(tg);
	if (!phone_number){
		if (callback){
			phone_number = 
					callback(userdata, TG_AUTH_PHONE_NUMBER, NULL);
		if (!phone_number)
			return 1;
		}
		// save to database
		phone_number_to_database(tg, phone_number);
	}
	printf("phone_number: %s\n", phone_number);

	// if not authorized start authorization
	// get tokens from database 
	buf_t tokens[20]; int tokens_len = 0;
	char *auth_tokens = auth_tokens_from_database(tg);
	if (auth_tokens){
		strtok_foreach(auth_tokens, ";", token){
			tokens[tokens_len++] = 
				buf_add((ui8_t*)token, strlen(token)); 
		}
	}
	
	// authorize with new key
  api.app.open();
  api.log.info(".. new session");
  shared_rc.ssid = api.buf.rand(8);
	api.srl.ping();
	
	CodeSettings codeSettings = tl_codeSettings(
			false,
		 	false,
		 	false,
		 	false,
		 	false, 
			false,
		 	auth_tokens ? tokens : NULL,
		 	tokens_len,
		 	NULL,
		 	NULL);

	printf("codeSettings:\n");
	buf_dump(codeSettings);

	buf_t sendCode = 
		tl_auth_sendCode(
				phone_number, 
				tg->apiId, 
				tg->apiHash, 
				&codeSettings);
	
	printf("sendCode:\n");
	buf_dump(sendCode);

	buf_t initConnection = 
		tl_initConnection(
				tg->apiId,
				"libtg", 
				"1.0", 
				"1.0", 
				"ru", 
				"LibTg", 
				"ru", 
				NULL, 
				NULL, 
				&sendCode);
	
	printf("initConnection:\n");
	buf_dump(initConnection);

	buf_t invokeWithLayer = 
		tl_invokeWithLayer(
				API_LAYER, &initConnection);

	buf_t answer = tl_send(invokeWithLayer); 
	if (!answer.size)
		return 1;
	/*printf("ANSWER: %.8x\n", id_from_tl_buf(answer));*/
	
	tl_t *tl = tl_deserialize(&answer);
	switch (tl->_id) {
		case id_auth_sentCode:
			{
				tl_auth_sentCode_t *tls = 
					(tl_auth_sentCode_t*) tl;
				// handle sent code
				if (callback){
					char *code = 
						callback(userdata, TG_AUTH_SENDCODE, tl);
					if (code){
						buf_t signIn = 
							tl_auth_signIn(
									phone_number, 
									tls->phone_code_hash_, 
									code, 
									NULL);
						buf_t answer = 
							tl_send(signIn); 
						printf("ANSWER: %.8x\n", 
								id_from_tl_buf(answer));
						tl_t *tl = tl_deserialize(&answer);
						/*free(code);*/
						switch (tl->_id) {
							case id_auth_authorization:
								{
									tl_auth_authorization_t *tla =
											(tl_auth_authorization_t *)tl;
									if (tla->setup_password_required_){
										if (callback)
											callback(userdata, TG_AUTH_PASSWORD_NEEDED, tl);
										return 1;
									}
									if (tla->future_auth_token_.size > 0){
										char auth_token[BUFSIZ];
										strncpy(
											auth_token,
											((char *)tla->future_auth_token_.data),
											tla->future_auth_token_.size);
										auth_token_to_database(tg, auth_token);
									}
									// save auth_key_id 
									auth_key_to_database(
											tg, shared_rc_get_key(), phone_number);
									if (callback)
										callback(userdata, TG_AUTH_SUCCESS, tla->user_);
									return 0;
								}
								break;
							
							default:
								break;
						}
					}
				}
				/*tl_auth_sentCode_free((tl_auth_sentCode_t *)tl);*/
			}
			break;
		
		default:
			break;
	}

	//free(phone_number);
	return 0;
}
