#ifndef TG_H_
#define TG_H_
#include <stdint.h>
#include <sqlite3.h>
#include <string.h>
#include "../libtg.h"
#include "../tl/str.h"

struct tg_ {
	int apiId;
	char apiHash[33];
	char database_path[BUFSIZ];
	const char *pubkey;
	sqlite3 *db;
	char ip[16];
	int port;
	int sockfd;
	bool net;
	void *on_err_data;
	void (*on_err)(void *on_err_data, tl_t *tl, const char *err);
	void *on_log_data;
	void (*on_log)(void *on_log_data, const char *msg);
	int seqn;
	buf_t key;
	buf_t salt;
	buf_t ssid;
	uint64_t fingerprint;
	uint64_t msgids[20]; 
};

int database_init(tg_t *tg, const char *database_path);
buf_t auth_key_from_database(tg_t *tg);
char * phone_number_from_database(tg_t *tg);
char * auth_tokens_from_database(tg_t *tg);

int phone_number_to_database(
		tg_t *tg, const char *phone_number);

int auth_token_to_database(
		tg_t *tg, const char *auth_token);

int auth_key_to_database(
		tg_t *tg, buf_t auth_key, const char *phone_number);

void tg_add_mgsid(tg_t*, uint64_t);


tl_t * tg_send_query_(tg_t *tg, buf_t s, bool encrypt);

#define ON_ERR(tg, tl, ...)\
	({if (tg->on_err){ \
		struct str _s; str_init(&_s); str_appendf(&_s, __VA_ARGS__);\
		tg->on_err(tg->on_err_data, tl, _s.str); \
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
