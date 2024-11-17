#include "../config.h"
#include "tg.h"
#include "../mtx/include/net.h"
#include <stdlib.h>
#include "strtok_foreach.h"

static buf_t _init(tg_t *tg, buf_t query)
{
	buf_t initConnection = 
		tl_initConnection(
				tg->apiId,
				PACKAGE_NAME, 
				PACKAGE_VERSION, 
				PACKAGE_VERSION, 
				"ru", 
				"LibTg", 
				"ru", 
				NULL, 
				NULL, 
				&query);
	//printf("initConnection:\n");
	//buf_dump(initConnection);
	
	buf_t invokeWithLayer = 
		tl_invokeWithLayer(
				API_LAYER, &initConnection);
	//printf("invokeWithLayer:\n");
	//buf_dump(invokeWithLayer);
	
	return invokeWithLayer;
}

tl_user_t * 
tg_is_authorized(tg_t *tg, void *on_err_data,
		void (*on_err)(void *on_err_data, tl_t *tl, const char *err))
{
	// try to load auth_key_id from database
	buf_t auth_key = auth_key_from_database(tg);

	if (auth_key.size){
		//printf("Have auth_key with len: %d\n", auth_key.size);
		reset_shared_rc();
		net_open(_ip, _port);
		shared_rc.salt = buf_rand(8);
		shared_rc.key = auth_key;
		shared_rc.seqnh = -1;
		api.log.info(".. new session");
		shared_rc.ssid = api.buf.rand(8);
		
		// check if authorized
		InputUser iuser = tl_inputUserSelf();
		printf("iuser:\n");
		buf_dump(iuser);
		
		buf_t getUsers = 
			tl_users_getUsers(&iuser, 1);	
		printf("getUsers:\n");
		buf_dump(getUsers);
		
		tl_t *tl = 
			tl_send(_init(tg, getUsers)); 
		
		if (tl && tl->_id == id_vector){
			tl_t *user = tl_deserialize(&((tl_vector_t *)tl)->data_);
			if (user && user->_id == id_user){
				return (tl_user_t *)user;
			}
		}
		// throw error
		char *err = tg_strerr(tl); 
		if (on_err)
			on_err(on_err_data, tl, err);
		free(err);
		// free tl
		/* TODO:  <14-11-24, kuzmich> */
		return NULL;
	}

	if (on_err)
		on_err(on_err_data, NULL, "NEED_TO_AUTHORIZE");
	return NULL;
}

tl_auth_sentCode_t *
tg_auth_sendCode(tg_t *tg, const char *phone_number, 
		void *on_err_data,
		void (*on_err)(void *on_err_data, tl_t *tl, const char *err))
{
	// get tokens from database 
	buf_t t[20]; int tn = 0;
	char *auth_tokens = auth_tokens_from_database(tg);
	if (auth_tokens){
		strtok_foreach(auth_tokens, ";", token){
			t[tn++] = 
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
		 	auth_tokens ? t : NULL,
		 	tn,
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
		tl_send(_init(tg, sendCode)); 

	if (tl && tl->_id == id_auth_sentCode){
		return (tl_auth_sentCode_t *)tl;
	}
	// throw error
	char *err = tg_strerr(tl); 
	if (on_err)
		on_err(on_err_data, tl, err);
	free(err);
	// free tl
	/* TODO:  <14-11-24, kuzmich> */

	return NULL;
}

tl_user_t *
tg_auth_signIn(tg_t *tg, tl_auth_sentCode_t *sentCode, 
		const char *phone_number, const char *phone_code, 
		void *on_err_data,
		void (*on_err)(void *on_err_data, tl_t *tl, const char *err))
{
	buf_t signIn = 
		tl_auth_signIn(
				phone_number, 
				(char *)sentCode->phone_code_hash_.data, 
				phone_code, 
				NULL);
	
	tl_t *tl = 
		tl_send(signIn);
	
	if (tl && tl->_id == id_auth_authorization){
		tl_auth_authorization_t *auth =
			(tl_auth_authorization_t *)tl;

		if (auth->setup_password_required_){
			// throw error
			if (on_err)
				on_err(on_err_data, tl, "SESSION_PASSWORD_NEEDED");
			return NULL;
		}
		
		if (auth->future_auth_token_.size > 0){
		// save auth token
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

		// save settings
		tg->rc = shared_rc;

		return (tl_user_t *)auth->user_;
	}

	// throw error
	char *err = tg_strerr(tl); 
	if (on_err)
		on_err(on_err_data, tl, err);
	free(err);
	// free tl
	/* TODO:  <14-11-24, kuzmich> */

	return NULL;
}
