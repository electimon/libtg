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

static int tg_sqlite3_exec(
		tg_t *tg, const char *sql) 
{
	printf("%s\n", sql);
	char *errmsg = NULL;

	int res = 
		sqlite3_exec(tg->db, sql, NULL, NULL, &errmsg);
	if (errmsg){
		// parse error
		perror(errmsg);
		sqlite3_free(errmsg);	
		return 1;
	}	
	if (res != SQLITE_OK){
		// parse error
		perror(sqlite3_errmsg(tg->db));
		return 1;
	}

	return 0;
}


ui64_t auth_key_id_from_database(tg_t *tg)
{
	if (!tg)
		return 0;

	char sql[] ="SELECT auth_key_id FROM auth;";
	
	sqlite3_stmt *stmt;
	if (tg_sqlite3_prepare(tg, sql, &stmt))
		return 0;
		
	ui64_t auth_key = 0;
	while (sqlite3_step(stmt) != SQLITE_DONE)
		auth_key = sqlite3_column_int64(stmt, 0);
	
	sqlite3_finalize(stmt);

	return auth_key;
}

char * phone_number_from_database(tg_t *tg)
{
	if (!tg)
		return 0;

	char sql[] ="SELECT phone_number FROM auth;";
	
	sqlite3_stmt *stmt;
	if (tg_sqlite3_prepare(tg, sql, &stmt))
		return 0;
		
	char *buf = NULL;
	while (sqlite3_step(stmt) != SQLITE_DONE)
		buf = (char *)sqlite3_column_text(stmt, 0);

	char *phone_number = strdup(buf);
	sqlite3_finalize(stmt);

	return phone_number;
}

int phone_number_to_database(
		tg_t *tg, const char *phone_number)
{
	char sql[BUFSIZ];
	/*sprintf(sql, */
			/*"CREATE TABLE IF NOT EXISTS auth (id INT); "*/
			/*"ALTER TABLE auth ADD COLUMN phone_number TEXT; "*/
			/*"INSERT INTO auth (phone_number) "*/
			/*"SELECT phone_number "*/
			/*"WHERE NOT EXISTS (SELECT 1 FROM auth); "*/
			/*"UPDATE auth SET phone_number = '%s'; "*/
		/*, phone_number);*/
	/*tg_sqlite3_exec(tg, sql);*/

	sprintf(sql, 
			"INSERT INTO auth (phone_number) "
			"SELECT phone_number "
			"WHERE NOT EXISTS (SELECT 1 FROM auth); "
			"UPDATE auth SET phone_number = '%s'; "
		, phone_number);
	tg_sqlite3_exec(tg, sql);

	return 0;
}
