#include "transport.h"
#include "stb_ds.h"
#include <pthread.h>

buf_t tg_ack(tg_t *tg)
{
	ON_LOG(tg, "%s", __func__);
	buf_t ret = buf_new();
	// make ack
	int err = pthread_mutex_lock(&tg->msgidsm);
	if (err){
		ON_ERR(tg, "%s: can't lock mutex: %d", __func__, err);
		return ret;
	}

	int len = arrlen(tg->msgids);
	if (len < 1)
		return ret;

	if (len > 20)
		len = 20;

	buf_t ack = tl_msgs_ack(tg->msgids, len);
	// remove from array
	tg->msgids = NULL;
	pthread_mutex_unlock(&tg->msgidsm);

	buf_free(ret);
	ret = tg_prepare_query(tg, ack, true, NULL);
	buf_free(ack);

	return ret;
}

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
