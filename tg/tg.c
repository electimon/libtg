#include "../libtg.h"
#include "../tl/alloc.h"
#include "tg.h"
#include "../transport/net.h"
#include "../transport/queue.h"
#include "../crypto/cry.h"
#include "../crypto/hsh.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../tl/serialize.h"

tg_t *tg_new(
		const char *database_path,
		int id,
		int apiId, 
		const char *apiHash, 
		const char *pem)
{
	if (!database_path)
		return NULL;

	// allocate struct
	tg_t *tg = NEW(tg_t, return NULL);	

	strncpy(tg->database_path,
		 	database_path, BUFSIZ-1);

	tg->id = id;
	
	// connect to SQL
	if (database_init(tg, database_path))
		return NULL;
	
	// set apiId and apiHash
	tg->apiId = apiId;
	strncpy(tg->apiHash, apiHash, 33);

	// set public_key
	tg->pubkey = pem;

	// set server address
	strncpy(tg->ip, SERVER_IP,
			sizeof(tg->ip) - 1);
	tg->port = SERVER_PORT;

	// set auth_key
	tg->key = auth_key_from_database(tg);
	if (tg->key.size){
		// auth key id
		buf_t key_hash = tg_hsh_sha1(tg->key);
		buf_t auth_key_id = 
			buf_add(key_hash.data + 12, 8);
		tg->key_id = buf_get_ui64(auth_key_id);
		buf_free(key_hash);
		buf_free(auth_key_id);
	}

	// start new seqn
	tg->seqn = 0;

	// load dialogs hash
	tg->dialogs_hash = dialogs_hash_from_database(tg);

	// start queue manager
	if (tg_start_send_queue_manager(tg))
		return NULL;
	if (tg_start_receive_queue_manager(tg))
		return NULL;

	return tg;
}

void tg_close(tg_t *tg)
{
	// close Telegram
	
	// close network
	tg_net_close(tg, tg->sockfd);
	
	// free
	free(tg);
}

void tg_set_on_error(tg_t *tg,
		void *on_err_data,
		void (*on_err)(void *on_err_data, const char *err))
{
	if (tg){
		tg->on_err = on_err;
		tg->on_err_data = on_err_data;
	}
}

void tg_set_on_log(tg_t *tg,
		void *on_log_data,
		void (*on_log)(void *on_log_data, const char *msg))
{
	if (tg){
		tg->on_log = on_log;
		tg->on_log_data = on_log_data;
	}
}

void tg_set_server_address(tg_t *tg, const char *ip, int port)
{
	if (tg){
		strncpy(tg->ip, ip,
			 	sizeof(tg->ip) - 1);
		tg->port = port;
	}
}

void tg_add_mgsid(tg_t *tg, uint64_t msgid){
	memmove(
			&(tg->msgids[1]), 
			tg->msgids, 
			sizeof(uint64_t)*19);
	tg->msgids[0] = msgid;
}

void update_hash(uint64_t *hash, uint32_t msg_id){
	int k;
	uint64_t h = 0;
	if (hash)
		h = *hash;

	h = h ^ (h >> 21);
	h = h ^ (h << 35);
	h = h ^ (h >> 4);
	h = h + msg_id;
	
	if (hash)
		*hash = h;
}

static buf_t 
tl_account_registerDevice_(
		bool no_muted_, 
		uint32_t token_type_, 
		unsigned char *token, 
		int token_len,
		Bool *app_sandbox_, 
		buf_t *secret_, 
		uint64_t *other_uids_, 
		int other_uids_len)
{
	buf_t buf = buf_add_ui32(0xec86017a);
	//parse argument flags ((null))
	uint32_t *flag1 = (uint32_t *)(&buf.data[buf.size]);
	buf = buf_cat_ui32(buf, 0);
	//parse argument no_muted (true)
	if (no_muted_)
		*flag1 |= (1 << 0);
	//parse argument token_type (int)
	{
		buf = buf_cat_ui32(buf, token_type_);
	}
	//parse argument token (string)
	{
		buf = buf_cat(buf, serialize_bytes(token, token_len));
	}
	//parse argument app_sandbox (Bool)
	{
		buf = buf_cat(buf, *app_sandbox_);
	}
	//parse argument secret (bytes)
	{
		buf = buf_cat(buf, serialize_bytes(secret_->data, secret_->size));
	}
	//parse argument other_uids (Vector<long>)
	{
		buf = buf_cat(buf, tl_vector());
		buf = buf_cat_ui32(buf, other_uids_len);
		int i;
		for (i=0; i<other_uids_len; ++i){
			buf = buf_cat_ui64(buf, other_uids_[i]);
		}
	}
	return buf;
}



int tg_account_register_ios(tg_t *tg, unsigned char *token, int token_len)
{
	int ret = 1;
	buf_t secret = buf_new();
	buf_t app_sandbox = tl_boolFalse();
	buf_t query = tl_account_registerDevice_(
			NULL, 
			1, 
			token, 
			token_len,
			&app_sandbox, 
			&secret, 
			NULL, 0); 
	buf_free(secret);
	buf_free(app_sandbox);
	
	tl_t *tl = tg_run_api(tg, &query);
	buf_free(query);
	
	if (tl == NULL)
		return 1;

	if (tl->_id == id_boolTrue)
		ret = 0;
	
	// free tl
	tl_free(tl);
	
	return ret;
}
