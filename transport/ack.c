#include "transport.h"
#include <pthread.h>

/*buf_t tg_ack(tg_t *tg, buf_t b)*/
/*{*/
	/*// make query*/
	/*buf_t query = tg_mtp_message(tg, b, true);*/
	
	/*// make ack*/
	/*int i;*/
	/*for (i = 0; i < 20; ++i) { // count msgs ids*/
		/*if (tg->msgids[i] == 0)*/
			/*break;	*/
	/*}*/
	/*buf_t buf = */
		/*tl_msgs_ack(tg->msgids, i);*/
	/*memset(tg->msgids, 0, sizeof(tg->msgids));*/

	/*buf_t ack = tg_mtp_message(tg, buf, false);*/
	/*buf_free(buf);	*/

	/*// make container*/
	/*buf_t msgs[2];*/
	/*msgs[0] = query;*/
	/*msgs[1] = ack;*/

	/*buf_t msg_container = */
		/*tl_msg_container(msgs, 2);*/
	/*buf_free(query);*/
	/*buf_free(ack);*/

	/*return msg_container;*/
/*}*/

buf_t tg_ack(tg_t *tg)
{
	// make ack
	pthread_mutex_lock(&tg->msgidsm);
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
