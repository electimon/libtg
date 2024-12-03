#include "../libtg.h"
#include "../tl/alloc.h"
#include "tg.h"
#include "../transport/net.h"
#include "../crypto/cry.h"
#include "../crypto/hsh.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

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
		void (*on_err)(void *on_err_data, tl_t *tl, const char *err))
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
