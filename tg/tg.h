#ifndef TG_H_
#define TG_H_
#include <pthread.h>
#include <stdint.h>
#include <sqlite3.h>
#include <string.h>
#include "../libtg.h"
#include "../tl/str.h"
#include "list.h"

struct tg_ {
	int id;
	int apiId;
	char apiHash[33];
	char database_path[BUFSIZ];
	const char *pubkey;
	char ip[16];
	int port;
	int sockfd;
	bool net;
	bool send_lock;
	list_t *send_queue;
	bool send_queue_lock;
	bool send_queue_manager;
	pthread_t send_queue_tid;
	list_t *receive_queue;
	bool receive_queue_lock;
	bool receive_queue_manager;
	pthread_t receive_queue_tid;
	void *on_err_data;
	void (*on_err)(void *on_err_data, const char *err);
	void *on_log_data;
	void (*on_log)(void *on_log_data, const char *msg);
	void *on_update_data;
	void (*on_update)(void *on_update_data, int type, void *data);
	int seqn;
	buf_t key;
	uint64_t key_id;
	buf_t salt;
	buf_t ssid;
	uint64_t fingerprint;
	uint64_t msgids[20]; 
	uint64_t dialogs_hash;
	bool sync_dialogs;
	int sync_dialogs_sockfd;
	pthread_t sync_dialogs_tid;
	uint64_t messages_hash;
	bool async_messages;
	int async_messages_sockfd;
	pthread_t async_messages_tid;
	int async_messages_seconds;
};

int database_init(tg_t *tg, const char *database_path);
buf_t auth_key_from_database(tg_t *tg);
char * phone_number_from_database(tg_t *tg);
char * auth_tokens_from_database(tg_t *tg);

void update_hash(uint64_t *hash, uint32_t msg_id);

uint64_t dialogs_hash_from_database(tg_t *tg);
int dialogs_hash_to_database(tg_t *tg, uint64_t hash);

uint64_t messages_hash_from_database(tg_t *tg, uint64_t peer_id);
int messages_hash_to_database(tg_t *tg, uint64_t peer_id, uint64_t hash);


char *photo_file_from_database(tg_t *tg, uint64_t photo_id);
int photo_to_database(tg_t *tg, uint64_t photo_id, const char *data);
char *peer_photo_file_from_database(tg_t *tg, 
		uint64_t peer_id, uint64_t photo_id);
int peer_photo_to_database(tg_t *tg, 
		uint64_t peer_id, uint64_t photo_id,
		const char *data);

int phone_number_to_database(
		tg_t *tg, const char *phone_number);

int auth_token_to_database(tg_t *tg, const char *auth_token);

int auth_key_to_database(
		tg_t *tg, buf_t auth_key);

void tg_add_mgsid(tg_t*, uint64_t);

tl_t * tg_send_query_(tg_t *tg, buf_t s, bool encrypt);
tl_t * tg_send_query_to_net(
		tg_t *tg, buf_t query, bool enc, int sockfd);

#define ON_UPDATE(tg, type, data)\
	({if (tg->on_update){ \
		tg->on_update(tg->on_update_data, type, data); \
	 }\
	})

#define ON_ERR(tg, ...)\
	({if (tg->on_err){ \
		struct str _s; str_init(&_s); str_appendf(&_s, __VA_ARGS__);\
		if (!strstr(_s.str, "duplicate column name:")) \
			tg->on_err(tg->on_err_data, _s.str); \
		free(_s.str);\
	 }\
	})

#define ON_LOG(tg, ...)\
	({if (tg->on_log){ \
		struct str _s; str_init(&_s); str_appendf(&_s, __VA_ARGS__);\
		tg->on_log(tg->on_log_data, _s.str); \
		free(_s.str);\
	 }\
	})

#define ON_LOG_BUF(tg, buf, ...)\
	({if (tg->on_log){ \
		struct str _s; str_init(&_s); str_appendf(&_s, __VA_ARGS__);\
		char *dump = buf_sdump(buf);\
		str_append(&_s, dump, strlen(dump));\
		free(dump);\
		tg->on_log(tg->on_log_data, _s.str); \
		free(_s.str);\
	 }\
	})

buf_t image_from_photo_stripped(buf_t photoStreppedData);

#endif /* ifndef TG_H_ */
