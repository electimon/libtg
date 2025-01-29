#include "tg.h"
#include "transport.h"
#include "stb_ds.h"
#include <pthread.h>

#if INTPTR_MAX == INT32_MAX
    #define THIS_IS_32_BIT_ENVIRONMENT
		#define _LD_ "%lld"
#elif INTPTR_MAX == INT64_MAX
    #define THIS_IS_64_BIT_ENVIRONMENT
		#define _LD_ "%ld"
#else
    #error "Environment not 32 or 64-bit."
#endif


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
	buf_t ack = buf_new();
	
	// send ACK
	int err = pthread_mutex_lock(&tg->msgidsm);
	if (err){
		ON_ERR(tg, "%s: can't lock mutex: %d", __func__, err);
		return ack;
	}

	int i, len = arrlen(tg->msgids);
	if (len < 1){
		// no messages to acknolage
		pthread_mutex_unlock(&tg->msgidsm);
		return ack;
	}

	for (i = 0; i < len; ++i) {
		ON_ERR(tg, "ACK: "_LD_"", tg->msgids[i]);
	}

	ack = tl_msgs_ack(
			tg->msgids, len);

	// free msgids
	arrfree(tg->msgids);
	tg->msgids = NULL;
	pthread_mutex_unlock(&tg->msgidsm);

	return ack;
}
