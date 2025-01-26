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

buf_t tg_ack(tg_t *tg)
{
	ON_LOG(tg, "%s", __func__);
	buf_t buf = buf_new();
	
	// send ACK
	int err = pthread_mutex_lock(&tg->msgidsm);
	if (err){
		ON_ERR(tg, "%s: can't lock mutex: %d", __func__, err);
		return buf;
	}

	int i, len = arrlen(tg->msgids);
	if (len < 1){
		// no messages to acknolage
		pthread_mutex_unlock(&tg->msgidsm);
		return buf;
	}

	buf_t ack = tl_msgs_ack(
			tg->msgids, len);
	buf_free(ack);

	// free msgids
	arrfree(tg->msgids);
	tg->msgids = NULL;
	pthread_mutex_unlock(&tg->msgidsm);

	return buf;
}
