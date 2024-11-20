#include "../config.h"
#include "tg.h"
#include "../mtx/include/net.h"
#include "../mtx/include/cmn.h"
#include "../mtx/include/hsh.h"
#include "../mtx/include/cry.h"
#include <stdlib.h>
#include "strtok_foreach.h"
#include "../tl/serialize.h"

static int _new_auth_key(tg_t *tg, void *on_err_data,
		void (*on_err)(void *on_err_data, tl_t *tl, const char *err))
{
	reset_shared_rc();
	net_open(_ip, _port);
	shared_rc.seqnh = -1;
	
	tl_t *tl = NULL;
	buf_t nonce = buf_rand(16);
	buf_t req_pq = tl_req_pq_multi(nonce);
	tl = tl_send_tl_message(req_pq, RFC);
	if (tl && tl->_id == id_resPQ){
		tl_resPQ_t *resPQ = (tl_resPQ_t *)tl;
		printf("server fingerprints:\n");
		int i;
		for (i = 0; i < resPQ->server_public_key_fingerprints_len; ++i) {
			printf("%.16lx\n", resPQ->server_public_key_fingerprints_[i]);
		}

		ui64_t pq_ = buf_get_ui64(buf_swap(resPQ->pq_));
		printf("PQ: %ld\n", pq_);

		ui32_t p_, q_;
		cmn_fact(pq_, &p_, &q_);
		if (!(p_ < q_)) {
			SWAP(p_, q_);
		}
		buf_t p  = buf_swap(buf_add_ui32(p_));
		buf_t q  = buf_swap(buf_add_ui32(q_));
		printf("P: %d\n", p_);
		printf("Q: %d\n", q_);

		buf_t pq_str = 
			serialize_bytes(resPQ->pq_.data, resPQ->pq_.size);
		buf_t p_str = 
			serialize_bytes(p.data, p.size);
		buf_t q_str = 
			serialize_bytes(q.data, q.size);
		
		buf_t new_nonce = buf_rand(32);

		buf_t dc  = buf_add_ui32(2);
		
		buf_t p_q_inner_data_dc = buf_add_ui32(id_p_q_inner_data_dc);
		/*tl_p_q_inner_data_dc(
		 * const char *pq_, 
		 * const char *p_, 
		 * const char *q_, 
		 * int128 nonce_, 
		 * int128 server_nonce_, 
		 * int256 new_nonce_, 
		 * int dc_);*/
		p_q_inner_data_dc = 
			buf_cat(p_q_inner_data_dc, pq_str);
		p_q_inner_data_dc = 
			buf_cat(p_q_inner_data_dc, p_str);
		p_q_inner_data_dc = 
			buf_cat(p_q_inner_data_dc, q_str);
		p_q_inner_data_dc = 
			buf_cat(p_q_inner_data_dc, nonce);
		p_q_inner_data_dc = 
			buf_cat(p_q_inner_data_dc, resPQ->server_nonce_);
		p_q_inner_data_dc = 
			buf_cat(p_q_inner_data_dc, new_nonce);
		p_q_inner_data_dc = 
			buf_cat(p_q_inner_data_dc, dc);

		buf_t h = hsh_sha1(p_q_inner_data_dc);
		buf_t dwh = buf_cat(h, p_q_inner_data_dc);	
		buf_t pad = {};
		buf_init(&pad);
		pad.size = 255 - dwh.size;
		dwh = buf_cat(dwh, pad);
		buf_t e = cry_rsa_e(dwh);
		buf_t encrypted_data = 
			serialize_bytes(e.data, e.size);
		
		buf_t public_key_fingerprint = 
			buf_swap(buf_add_ui64(resPQ->server_public_key_fingerprints_[0]));
		printf("fingerprint: %.16lx\n", *(long*)public_key_fingerprint.data);
		buf_t req_DH_params = buf_add_ui32(id_req_DH_params);
		//buf_t req_DH_params = 
			//tl_req_DH_params(
					//int128 nonce_, 
					//int128 server_nonce_, 
					//const char *p_, 
					//const char *q_, 
					//resPQ->server_public_key_fingerprints_[0], 
					//const char *encrypted_data_);
		req_DH_params = buf_cat(req_DH_params, nonce);
		req_DH_params = 
			buf_cat(req_DH_params, resPQ->server_nonce_);
		req_DH_params = buf_cat(req_DH_params, p_str);
		req_DH_params = buf_cat(req_DH_params, q_str);
		req_DH_params = 
			buf_cat(req_DH_params, public_key_fingerprint);
		req_DH_params = buf_cat(req_DH_params, encrypted_data);

		tl = tl_send_tl_message(req_DH_params, RFC);
		printf("ANSW: %.8x\n", tl->_id);

		return 0;
	}

	// throw error
	char *err = tg_strerr(tl); 
	if (on_err)
		on_err(on_err_data, tl, err);
	free(err);

	return 1;
}

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
	//api.app.open();
	/*api.log.info(".. new session");*/
	/*shared_rc.ssid = api.buf.rand(8);*/
	/*api.srl.ping();*/
	if (_new_auth_key(tg, on_err_data, on_err))
		return NULL;
	
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
