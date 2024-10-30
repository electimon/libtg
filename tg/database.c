#include "tg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int tg_sqlite3_prepare(
		tg_t *tg, const char *sql, sqlite3_stmt **stmt) 
{
	int res = sqlite3_prepare_v2(
			tg->db, 
			sql, 
			-1, 
			stmt,
		 	NULL);
	if (res != SQLITE_OK){
		// parse error
		perror(sqlite3_errmsg(tg->db));
		return 1;
	}	

	return 0;
}

ui64_t auth_key_id_from_database(
		tg_t *tg, const char *phone_number)
{
	if (!tg)
		return 0;

	char *pn = strdup(phone_number);
	// remove '+'
	char *pnn = pn + 1;

	char sql[BUFSIZ];
	sprintf(sql, "SELECT auth_key_id FROM users"
			" WHERE phone_number = '%s';", pnn);
	
	sqlite3_stmt *stmt;
	if (tg_sqlite3_prepare(tg, sql, &stmt))
		return 0;
		
	ui64_t auth_key = 0;
	while (sqlite3_step(stmt) != SQLITE_DONE)
		auth_key = sqlite3_column_int64(stmt, 0);
	
	sqlite3_finalize(stmt);

	free(pn);
	
	return auth_key;
}
