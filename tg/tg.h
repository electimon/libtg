#ifndef TG_H_
#define TG_H_
#include <sqlite3.h>
#include "../libtg.h"
#include "../mtx/include/api.h"

struct tg_ {
	app_t app;
	int apiId;
	char apiHash[32];
	sqlite3 *db;
};

ui64_t auth_key_id_from_database(tg_t *tg);
char * phone_number_from_database(tg_t *tg);

int phone_number_to_database(
		tg_t *tg, const char *phone_number);

#endif /* ifndef TG_H_ */
