#include "tg.h"
#include "str.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char tg_database_path[BUFSIZ];

static int tg_sqlite3_open(tg_t *tg) 
{
	int err = sqlite3_open(
			tg_database_path, &tg->db);
	if (err){
		api.log.error((char *)sqlite3_errmsg(tg->db));
		return 1;
	}

	return 0;
}

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

#define tg_sqlite3_for_each(tg, sql, stmt) \
	sqlite3_stmt *stmt;\
	int sqlite_step;\
	if (tg_sqlite3_open(tg) == 0)\
		if (tg_sqlite3_prepare(tg, sql, &stmt) == 0)\
			for (sqlite_step = sqlite3_step(stmt);\
					sqlite_step	!= SQLITE_DONE || ({sqlite3_finalize(stmt); sqlite3_close(tg->db); 0;});\
					sqlite_step = sqlite3_step(stmt))\
			 

static int tg_sqlite3_exec(
		tg_t *tg, const char *sql) 
{
	if (tg_sqlite3_open(tg))
		return 1;

	/*printf("%s\n", sql);*/
	char *errmsg = NULL;

	int res = 
		sqlite3_exec(tg->db, sql, NULL, NULL, &errmsg);
	if (errmsg){
		// parse error
		perror(errmsg);
		sqlite3_free(errmsg);	
		sqlite3_close(tg->db);
		return 1;
	}	
	if (res != SQLITE_OK){
		// parse error
		perror(sqlite3_errmsg(tg->db));
		sqlite3_close(tg->db);
		return 1;
	}

	sqlite3_close(tg->db);
	return 0;
}

int database_init(tg_t *tg, const char *database_path)
{
	strncpy(tg_database_path, database_path, BUFSIZ-1);
	tg_database_path[BUFSIZ-1] = 0;

	int err = sqlite3_open_v2(
			tg_database_path, &tg->db, 
			SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 
			NULL);
	if (err){
		api.log.error((char *)sqlite3_errmsg(tg->db));
		return 1;
	}

	sqlite3_close(tg->db);
	return 0;
}

buf_t auth_key_from_database(tg_t *tg)
{
	char sql[] ="SELECT auth_key_id FROM auth WHERE id = 0;";
	buf_t auth_key;
	memset(&auth_key, 0, sizeof(buf_t));
	tg_sqlite3_for_each(tg, sql, stmt){
		auth_key = buf_add(
			(ui8_t*)sqlite3_column_blob(stmt, 0),
			sqlite3_column_bytes(stmt, 0));
	}
	
	return auth_key;
}

char * phone_number_from_database(tg_t *tg)
{
	char sql[] ="SELECT phone_number FROM auth WHERE id = 0;";
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
			"CREATE TABLE IF NOT EXISTS auth (id INT); "
			"ALTER TABLE \'auth\' ADD COLUMN \'phone_number\' TEXT; "
			"INSERT INTO \'auth\' (\'phone_number\') "
			"SELECT \'phone_number\' "
			"WHERE NOT EXISTS (SELECT 1 FROM auth); "
			"UPDATE \'auth\' SET \'phone_number\' = \'%s\', id = 0; "
		, phone_number);
	return tg_sqlite3_exec(tg, sql);
}

char * auth_tokens_from_database(tg_t *tg)
{
	char sql[] = 
		"SELECT * FROM ((SELECT ROW_NUMBER() OVER (ORDER BY ID) "
		"AS Number, auth_token FROM auth)) ORDER BY Number DESC "	
		"LIMIT 20;";
	struct str s;
	if (str_init(&s))
		return NULL;

	int i = 0;
	tg_sqlite3_for_each(tg, sql, stmt){
		if (i++ > 0)
			str_append(&s, ";", 1);
		str_append(&s, 
				(char *)sqlite3_column_text(stmt, 1),
				sqlite3_column_bytes(stmt, 1));
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
			"CREATE TABLE IF NOT EXISTS auth (id INT); "
			"ALTER TABLE \'auth\' ADD COLUMN \'auth_token\' TEXT; "
			"INSERT INTO \'auth\' (\'phone_number\') VALUES (\'%s\')"
		, auth_token);
	return tg_sqlite3_exec(tg, sql);
}

int auth_key_to_database(
		tg_t *tg, buf_t auth_key)
{
	char sql[BUFSIZ] =
			"CREATE TABLE IF NOT EXISTS auth (id INT); "
			"ALTER TABLE \'auth\' ADD COLUMN \'auth_key\' BLOB; "
			"INSERT INTO \'auth\' (\'auth_key\') "
			"SELECT \'auth_key\' "
			"WHERE NOT EXISTS (SELECT 1 FROM auth); ";
	
	int res = sqlite3_open(tg_database_path, &tg->db);
	if (res){
		printf("%s\n", sqlite3_errmsg(tg->db));
		return 1;
	}
	char *errmsg = NULL;
	res = sqlite3_exec(tg->db, sql, NULL, NULL, &errmsg);
	if (errmsg) {
		perror(errmsg);
		free(errmsg);
		return 1;
	}	
			
	strcpy(sql, 
			"UPDATE \'auth\' SET \'auth_key\' = (?), id = 0; ");
	
	sqlite3_stmt *stmt;
	res = sqlite3_prepare_v2(tg->db, sql, -1, &stmt, NULL);
	if (res != SQLITE_OK) {
		perror(errmsg);
		free(errmsg);
		return 1;
	}	

	res = sqlite3_bind_blob(stmt, 1, auth_key.data, auth_key.size, SQLITE_TRANSIENT);
	if (res != SQLITE_OK) {
		perror(sqlite3_errmsg(tg->db));		
	}	
	sqlite3_finalize(stmt);
	sqlite3_close(tg->db);

	return 0;
}
