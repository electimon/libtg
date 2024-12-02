#include "messages.h"

int tg_messages_getHistory(
		tg_t *tg,
		buf_t *peer,
		int offset_id,
		int offset_date,
		int add_offset,
		int limit,
		int max_id,
		int min_id,
		long *hash,
		void *data,
		int (*callback)(void *data, 
			const tg_message_t *message))
{
	int i, k;
	long h = 0;
	if (hash)
		h = *hash;

	buf_t getHustory = 
		tl_messages_getHistory(
				peer, 
				offset_id, 
				offset_date, 
				add_offset, 
				limit, 
				max_id, 
				min_id, 
				h);

	tl_t *tl = tg_send_query_to_net(
			tg, getHustory, 
			true, tg->async_messages_sockfd);

	if (tl && tl->_id == id_messages_messages){

	} else { // not messages
		// throw error
		char *err = tg_strerr(tl); 
		ON_ERR(tg, tl, "%s", err);
		free(err);
		// free tl
		/* TODO:  <29-11-24, yourname> */
	}

	return 0;
}

