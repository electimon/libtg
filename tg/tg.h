#ifndef TG_H_
#define TG_H_
#include <sqlite3.h>
#include "../libtg.h"
#include "../mtx/include/api.h"

struct tg_ {
	app_t app;
	int apiId;
	char apiHash[33];
	char database_path[BUFSIZ];
	sqlite3 *db;
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
#endif /* ifndef TG_H_ */
