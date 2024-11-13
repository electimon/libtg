#include "tg.h"
#include "strtok_foreach.h"
#include <stdio.h>
#include <string.h>
#include "../mtx/include/net.h"
#include <byteswap.h>

static buf_t init(tg_t *tg, buf_t query)
{
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
				&query);
	
	buf_t invokeWithLayer = 
		tl_invokeWithLayer(
				API_LAYER, &initConnection);
	
	return invokeWithLayer;
}

int tg_connect(
		tg_t *tg,
		void *userdata,
		char * (*callback)(
			void *userdata,
			TG_AUTH auth,
			const tl_t *tl,
			const char *error))
{
	// try to load auth_key_id from database
	buf_t auth_key = auth_key_from_database(tg);

	if (auth_key.size){
		printf("Have auth_key with len: %d\n", auth_key.size);
		reset_shared_rc();
		net_open(_ip, _port);
		shared_rc.salt = buf_rand(8);
		shared_rc.key = auth_key;
		shared_rc.seqnh = -1;
		api.log.info(".. new session");
		shared_rc.ssid = api.buf.rand(8);
		
		// check if authorized
		InputUser iuser = tl_inputUserSelf();
		buf_t getUsers = 
			tl_users_getUsers(&iuser, 1);	
		tl_t *tl = 
			tl_send(init(tg, getUsers)); 
		
		if (tl && tl->_id == id_vector){
			tl_t *user = tl_deserialize(&((tl_vector_t *)tl)->data_);
			if (user && user->_id == id_user){
				if (callback)
					callback(userdata, TG_AUTH_SUCCESS, user, NULL);
				return 0;
			}
		}
	}

	// try to get phone_number
	char * phone_number = phone_number_from_database(tg);
	if (!phone_number){
		if (callback){
			phone_number = 
					callback(userdata, TG_AUTH_PHONE_NUMBER, NULL, NULL);
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

	//printf("codeSettings:\n");
	//buf_dump(codeSettings);

	buf_t sendCode = 
		tl_auth_sendCode(
				phone_number, 
				tg->apiId, 
				tg->apiHash, 
				&codeSettings);
	
	//printf("sendCode:\n");
	//buf_dump(sendCode);

	tl_t *tl = 
		tl_send(init(tg, sendCode)); 
	if (!tl){
		if (callback)
			callback(userdata, TG_AUTH_ERROR, NULL, "can't deserialize tl_send");	
		return 1;
	}
	
	switch (tl->_id) {
		case id_auth_sentCode:
			{
				tl_auth_sentCode_t *tls = 
					(tl_auth_sentCode_t*) tl;
				// handle sent code
				if (callback){
					char *code = 
						callback(userdata, TG_AUTH_SENDCODE, tl, NULL);
					if (!code){
						if (callback)
							callback(userdata, TG_AUTH_ERROR, NULL, 
									"phone code is NULL");
						return 1;
					}
					buf_t signIn = 
						tl_auth_signIn(
								phone_number, 
								string_from_buf(
									tls->phone_code_hash_), 
								code, 
								NULL);
					tl_t *tl = 
						tl_send(signIn); 
					if (!tl){
						if (callback)
							callback(userdata, TG_AUTH_ERROR, NULL, 
									"can't deserialize tl_send");	
						return 1;
					}

					/*free(code);*/
					switch (tl->_id) {
						case id_rpc_error:
							{
								tl_rpc_error_t *error = 
									(tl_rpc_error_t *)tl;
								if (callback)
									callback(userdata, TG_AUTH_ERROR, tl,
											(char *)error->error_message_.data);

								return 1;
							}
						case id_bad_msg_notification:
							{
								tl_bad_msg_notification_t *bmsgn = 
									(tl_bad_msg_notification_t *)tl;
								char err[BUFSIZ];
								sprintf(err, 
										"bad msg notification: %d", 
										bmsgn->error_code_);
								if (callback)
									callback(userdata, TG_AUTH_ERROR, tl,
											err);

								return 1;
							}
						case id_auth_authorization:
							{
								tl_auth_authorization_t *auth =
										(tl_auth_authorization_t *)tl;
								if (auth->setup_password_required_){
									if (callback)
										callback(userdata, 
												TG_AUTH_PASSWORD_NEEDED, tl, NULL);
									return 1;
								}
								if (auth->future_auth_token_.size > 0){
									char auth_token[BUFSIZ];
									strncpy(
										auth_token,
										((char *)auth->future_auth_token_.data),
										auth->future_auth_token_.size);
									auth_token_to_database(tg, auth_token);
								}
								// save auth_key_id 
								auth_key_to_database(
										tg, shared_rc_get_key(),
									 	phone_number);
								if (callback)
									callback(userdata, 
											TG_AUTH_SUCCESS, auth->user_, NULL);
								return 0;
							}
						
						default:
							break;
					}
				/*tl_auth_sentCode_free((tl_auth_sentCode_t *)tl);*/
				}
			}
		default:
			{
				if (callback)
					callback(userdata, TG_AUTH_ERROR, tl, 
							"unknown response");
				break;
			}
		}

	//free(phone_number);
	return 1;
}
