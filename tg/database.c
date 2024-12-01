#include "tg.h"
#include "../tl/str.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"

sqlite3 * tg_sqlite3_open(tg_t *tg) 
{
	sqlite3 *db = NULL;
	int err = sqlite3_open_v2(
			tg->database_path,
		 	&db, 
			SQLITE_OPEN_READWRITE | SQLITE_OPEN_NOMUTEX, 
			NULL);
	if (err){
		ON_ERR(tg, NULL, "%s", (char *)sqlite3_errmsg(db));
		return NULL;
	}

	return db;
}

int tg_sqlite3_prepare(
		tg_t *tg, sqlite3 *db, const char *sql, sqlite3_stmt **stmt) 
{
	int res = sqlite3_prepare_v2(
			db, 
			sql, 
			-1, 
			stmt,
		 	NULL);
	if (res != SQLITE_OK){
		// parse error
		ON_ERR(tg, NULL, "%s", sqlite3_errmsg(db));
		return 1;
	}	

	return 0;
}

int tg_sqlite3_exec(
		tg_t *tg, const char *sql) 
{
	sqlite3 *db =	tg_sqlite3_open(tg);
	if (!db)
		return 1;

	char *errmsg = NULL;

	int res = 
		sqlite3_exec(db, sql, NULL, NULL, &errmsg);
	if (errmsg){
		// parse error
		ON_ERR(tg, NULL, "%s", errmsg);
		sqlite3_free(errmsg);	
		sqlite3_close(db);
		return 1;
	}	
	if (res != SQLITE_OK){
		// parse error
		ON_ERR(tg, NULL, "%s", sqlite3_errmsg(db));
		sqlite3_close(db);
		return 1;
	}

	sqlite3_close(db);
	return 0;
}

int database_init(tg_t *tg, const char *database_path)
{
	sqlite3 *db;
	int err = sqlite3_open_v2(
			tg->database_path, &db, 
			SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, 
			NULL);
	if (err){
		ON_ERR(tg, NULL, "%s", (char *)sqlite3_errmsg(db));
		return 1;
	}

	sqlite3_close(db);
	return 0;
}

buf_t auth_key_from_database(tg_t *tg)
{
	char sql[BUFSIZ];
	sprintf(sql, 
			"SELECT auth_key FROM auth_keys WHERE id = %d;"
			, tg->id);
	buf_t auth_key;
	memset(&auth_key, 0, sizeof(buf_t));
	tg_sqlite3_for_each(tg, sql, stmt){
		auth_key = buf_add(
			(uint8_t*)sqlite3_column_blob(stmt, 0),
			sqlite3_column_bytes(stmt, 0));
	}
	
	return auth_key;
}

char * phone_number_from_database(tg_t *tg)
{
	char sql[BUFSIZ];
	sprintf(sql, 
			"SELECT phone_number FROM phone_numbers WHERE id = %d;"
			, tg->id);
	char buf[BUFSIZ] = {0};
	tg_sqlite3_for_each(tg, sql, stmt)
		strcpy(buf, (char *)sqlite3_column_text(stmt, 0));

	if (*buf)
		return strdup(buf);
	else
		return NULL;
}

int phone_number_to_database(
		tg_t *tg, const char *phone_number)
{
	char sql[BUFSIZ];

	sprintf(sql, 
			"CREATE TABLE IF NOT EXISTS phone_numbers (id INT); "
			"ALTER TABLE \'phone_numbers\' ADD COLUMN \'phone_number\' TEXT; "
			"INSERT INTO \'phone_numbers\' (\'id\') "
			"SELECT %d "
			"WHERE NOT EXISTS (SELECT 1 FROM phone_numbers WHERE id = %d); "
			"UPDATE \'phone_numbers\' SET \'phone_number\' = \'%s\', id = %d; "
		,tg->id, tg->id, phone_number, tg->id);
	return tg_sqlite3_exec(tg, sql);
}

char * auth_tokens_from_database(tg_t *tg)
{
	char sql[BUFSIZ];
	sprintf(sql, 
		"SELECT * FROM ((SELECT ROW_NUMBER() OVER (ORDER BY ID) "
		"AS Number, auth_token FROM auth_tokens)) WHERE id = %d "
		"ORDER BY Number DESC "	
		"LIMIT 20;", tg->id);
	struct str s;
	if (str_init(&s))
		return NULL;

	int i = 0;
	tg_sqlite3_for_each(tg, sql, stmt){
		if (i > 0)
			str_append(&s, ";", 1);
		if (sqlite3_column_bytes(stmt, 1) > 0){
			str_append(&s, 
					(char *)sqlite3_column_text(stmt, 1),
					sqlite3_column_bytes(stmt, 1));
			i++;
		}
	}

	if (s.len){
		return s.str;
	} else{
		free(s.str);
		return NULL;
	}
}

int auth_token_to_database(
		tg_t *tg, const char *auth_token)
{
	char sql[BUFSIZ];

	sprintf(sql, 
			"CREATE TABLE IF NOT EXISTS auth_tokens (id INT); "
			"ALTER TABLE \'auth_tokens\' ADD COLUMN \'auth_token\' TEXT; "
			"INSERT INTO \'auth_tokens\' (id, \'auth_token\') VALUES (%d, \'%s\'); "
		, tg->id, auth_token);
	return tg_sqlite3_exec(tg, sql);
}

int auth_key_to_database(
		tg_t *tg, buf_t auth_key)
{
	char sql[BUFSIZ];
	sprintf(sql, 
			"CREATE TABLE IF NOT EXISTS auth_keys (id INT); "
			"ALTER TABLE \'auth_keys\' ADD COLUMN \'auth_key\' BLOB; "
			"INSERT INTO \'auth_keys\' (id) "
			"SELECT %d "
			"WHERE NOT EXISTS (SELECT 1 FROM auth_keys WHERE id = %d); "
			, tg->id, tg->id);
	
	sqlite3 *db = tg_sqlite3_open(tg);
	char *errmsg = NULL;
	int res = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
	if (errmsg) {
		ON_ERR(tg, NULL, "%s", errmsg);
		free(errmsg);
		sqlite3_close(db);
		return 1;
	}	
			
	sprintf(sql, 
			"UPDATE \'auth_keys\' SET \'auth_key\' = (?) "
			"WHERE id = %d; ", tg->id);
	
	sqlite3_stmt *stmt;
	res = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	if (res != SQLITE_OK) {
		ON_ERR(tg, NULL, "%s", errmsg);
		free(errmsg);
		sqlite3_close(db);
		return 1;
	}	

	res = sqlite3_bind_blob(stmt, 1, auth_key.data, auth_key.size, SQLITE_TRANSIENT);
	if (res != SQLITE_OK) {
		ON_ERR(tg, NULL, "%s", sqlite3_errmsg(db));
	}	
	
	sqlite3_step(stmt);
	
	sqlite3_finalize(stmt);
	sqlite3_close(db);

	return 0;
}

long dialogs_hash_from_database(tg_t *tg)
{
	char sql[BUFSIZ];
	sprintf(sql, 
			"SELECT hash FROM dialogs_hash WHERE id = %d;"
			, tg->id);
	long hash;
	tg_sqlite3_for_each(tg, sql, stmt)
		hash = sqlite3_column_int64(stmt, 0);

	return hash;
}

int dialogs_hash_to_database(tg_t *tg, long hash)
{
	char sql[BUFSIZ];
	sprintf(sql, 
			"CREATE TABLE IF NOT EXISTS dialogs_hash (id INT); "
			"ALTER TABLE \'dialogs_hash\' ADD COLUMN \'hash\' INT; "
			"INSERT INTO \'dialogs_hash\' (\'id\') "
			"SELECT %d "
			"WHERE NOT EXISTS (SELECT 1 FROM dialogs_hash WHERE id = %d); "
			"UPDATE \'dialogs_hash\' SET \'hash\' = \'%ld\', id = %d; "
		,tg->id, tg->id, hash, tg->id);
	
	return tg_sqlite3_exec(tg, sql);
}

