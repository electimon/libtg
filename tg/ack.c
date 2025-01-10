#include "transport.h"
#include <pthread.h>

buf_t tg_ack(tg_t *tg)
{
	ON_LOG(tg, "%s", __func__);
	// make ack
	int err = pthread_mutex_lock(&tg->msgidsm);
	if (err){
		ON_ERR(tg, "%s: can't lock mutex: %d", __func__, err);
	}
	int i;
	for (i = 0; i < 20; ++i) { // count msgs ids
		if (tg->msgids[i] == 0)
			break;	
	}
	buf_t ack = 
		tl_msgs_ack(tg->msgids, i);
	memset(tg->msgids, 0, sizeof(tg->msgids));
	pthread_mutex_unlock(&tg->msgidsm);

	buf_t h = tg_header(tg, ack, true, false, NULL);
	buf_t e = tg_encrypt(tg, h, true);
	buf_t t = tg_transport(tg, e);

	return t;
}
