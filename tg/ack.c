#include "transport.h"
#include "stb_ds.h"
#include <pthread.h>

void tg_add_msgid(tg_t *tg, uint64_t msgid){
	ON_LOG(tg, "%s", __func__);
	int err = pthread_mutex_lock(&tg->msgidsm);
	if (err){
		ON_ERR(tg, "%s: can't lock mutex: %d", __func__, err);
		return;
	}
	arrput(tg->msgids, msgid);
	pthread_mutex_unlock(&tg->msgidsm);
}
