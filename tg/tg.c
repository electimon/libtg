#include "../libtg.h"
#include "tg.h"
#include "../mtx/include/api.h"
#include "../tl/alloc.h"
#include "../mtx/include/net.h"
#include "../mtx/include/srl.h"
#include "../mtx/include/app.h"
#include <stdio.h>
#include <string.h>

tg_t *tg_new(const char *database_path,
		int apiId, const char *apiHash)
{
	if (!database_path)
		return NULL;

	// allocate struct
	tg_t *tg = NEW(tg_t, return NULL);	

	strncpy(tg->database_path,
		 	database_path, BUFSIZ-1);
	
	// connect to SQL
	if (database_init(tg, database_path))
		return NULL;
	
	// set apiId and apiHash
	tg->apiId = apiId;
	strncpy(tg->apiHash, apiHash, 33);

	return tg;
}

void tg_close(tg_t *tg)
{
	// close Telegram
	
	// close mtproto
  net_t n = shared_rc_get_net();
  api.net.close(n);
	
	// close database
	sqlite3_close(tg->db);
	
	// free
	free(tg);
}
