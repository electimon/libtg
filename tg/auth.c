#include "../config.h"
#include "tg.h"
#include "../transport/net.h"
#include <stdio.h>
#include <stdlib.h>
#include "strtok_foreach.h"
#include "../tl/serialize.h"
#include "../mtx/include/types.h"
#include "../mtx/include/api.h"
#include <sys/socket.h>

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
	
	ON_LOG_BUF(tg, initConnection, 
			"%s: initConnection: ", __func__);
	
	buf_t invokeWithLayer = 
		tl_invokeWithLayer(
				API_LAYER, &initConnection);
	
	ON_LOG_BUF(tg, invokeWithLayer, 
			"%s: invokeWithLayer: ", __func__);
	
	return invokeWithLayer;
}

tl_user_t *
tg_is_authorized(tg_t *tg)
{
	if (tg->key.size){
		ON_LOG(tg, "have auth_key with len: %d", tg->key.size);

		// check if authorized
		InputUser iuser = tl_inputUserSelf();
		ON_LOG_BUF(tg, iuser, 
				"%s: InputUser: ", __func__);
		
		buf_t getUsers = 
			tl_users_getUsers(&iuser, 1);	
		ON_LOG_BUF(tg, getUsers, 
				"%s: getUsers: ", __func__);
		
		tl_t *tl = 
			tg_send_query(tg, _init(tg, getUsers)); 
		
		if (tl && tl->_id == id_vector){
			tl_t *user = tl_deserialize(&((tl_vector_t *)tl)->data_);
			if (user && user->_id == id_user){
				return (tl_user_t *)user;
			}
		}
		// throw error
		char *err = tg_strerr(tl); 
		ON_ERR(tg, "%s", err);
		free(err);
		// free tl
		/* TODO:  <14-11-24, kuzmich> */
		return NULL;
	}

	ON_ERR(tg, "NEED_TO_AUTHORIZE");
	return NULL;
}

tl_auth_sentCode_t *
tg_auth_sendCode(tg_t *tg, const char *phone_number) 
{
	// create new auth_key
	if (tg->net){
		tg_net_close(tg, tg->sockfd);
	}

	api.app.open();
	tg->sockfd = shared_rc.net.sockfd;
	tg->net = true;
	tg->key = 
		buf_add(shared_rc.key.data, shared_rc.key.size);
	tg->salt = 
		buf_add(shared_rc.salt.data, shared_rc.salt.size);
	tg->ssid = buf_rand(8);
	tg->seqn = shared_rc.seqnh + 1;

	// get tokens from database 
	buf_t t[20]; int tn = 0;
	char *auth_tokens = auth_tokens_from_database(tg);
	if (auth_tokens){
		strtok_foreach(auth_tokens, ";", token){
			t[tn++] = 
				buf_add((uint8_t*)token, strlen(token)); 
		}
	}
	
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

  	ON_LOG_BUF(tg, codeSettings, 
			"%s: codeSettings: ", __func__);

	buf_t sendCode = 
		tl_auth_sendCode(
				phone_number, 
				tg->apiId, 
				tg->apiHash, 
				&codeSettings);
	
	ON_LOG_BUF(tg, sendCode, 
			"%s: sendCode: ", __func__);

	tl_t *tl = 
		tg_send_query(tg, _init(tg, sendCode)); 

	if (tl && tl->_id == id_auth_sentCode){
		return (tl_auth_sentCode_t *)tl;
	}
	// throw error
	char *err = tg_strerr(tl); 
	ON_ERR(tg, "%s", err);
	free(err);
	// free tl
	/* TODO:  <14-11-24, kuzmich> */

	return NULL;
}

tl_user_t *
tg_auth_signIn(tg_t *tg, tl_auth_sentCode_t *sentCode, 
		const char *phone_number, const char *phone_code) 
{
	buf_t signIn = 
		tl_auth_signIn(
				phone_number, 
				(char *)sentCode->phone_code_hash_.data, 
				phone_code, 
				NULL);
	
	tl_t *tl = 
		tg_send_query(tg, signIn);
	
	if (tl && tl->_id == id_auth_authorization){
		tl_auth_authorization_t *auth =
			(tl_auth_authorization_t *)tl;

		if (auth->setup_password_required_){
			// throw error
			ON_ERR(tg, "SESSION_PASSWORD_NEEDED");
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
		auth_key_to_database(tg, tg->key);

		// close port
		//tg_net_close(tg, tg->sockfd);
		//tg->net = false;

		return (tl_user_t *)auth->user_;
	}

	// throw error
	char *err = tg_strerr(tl); 
	ON_ERR(tg, "%s", err);
	free(err);
	// free tl
	/* TODO:  <14-11-24, kuzmich> */

	return NULL;
}
