#include "messages.h"
#include <string.h>
#include "peer.h"
#include "tg.h"

void tg_message_from_tl(
		tg_message_t *tgm, tl_message_t *tlm)
{
	#define TG_MESSAGE_ARG(t, arg, ...) \
		tgm->arg = tlm->arg;
	#define TG_MESSAGE_STR(t, arg, ...) \
		tgm->arg = strndup((char*)tlm->arg.data, tlm->arg.size);
	#define TG_MESSAGE_PER(t, arg, ...) \
	if (tlm->arg){\
		tl_peerUser_t *peer = (tl_peerUser_t *)tlm->arg; \
		tgm->arg = peer->user_id_;\
		tgm->type_##arg = peer->_id;\
	}
	TG_MESSAGE_ARGS
	#undef TG_MESSAGE_ARG
	#undef TG_MESSAGE_STR
	#undef TG_MESSAGE_PER
}

static void parse_msg(
		int *c, tg_message_t *tgm, void *tlm_)
{
	tg_message_from_tl(tgm, tlm_);
	*c += 1;
}

static void parse_msgs(
		tg_t *tg, int *c, 
		int argc, tl_t **argv,
		void *data,
		int (*callback)(void *data, 
			const tg_message_t *message))
{
	int i;
	for (i = 0; i < argc; ++i) {
		if (!argv[i] || argv[i]->_id != id_message)
			continue;
				
		tg_message_t m;
		memset(&m, 0, sizeof(m));
		parse_msg(c, &m, argv[i]);
		if (callback)
			if (callback(data, &m))
				break;
	}
}	

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
	int i, k, c = 0;
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

	ON_LOG(tg, "%s: recived id: %.8x", __func__, tl->_id);
	if (!tl)
		goto tg_messeges_get_history_thow_error;

	switch (tl->_id) {
		case id_messages_channelMessages:
			{
				tl_messages_channelMessages_t *msgs = 
					(tl_messages_channelMessages_t *)tl;
				
				parse_msgs(
						tg, &c, 
						msgs->messages_len, 
						msgs->messages_, 
						data, 
						callback);

				goto tg_messeges_get_history_finish;
			}
			break;
			
		case id_messages_messages:
			{
				tl_messages_messages_t *msgs = 
					(tl_messages_messages_t *)tl;
				
				parse_msgs(
						tg, &c, 
						msgs->messages_len, 
						msgs->messages_, 
						data, 
						callback);

				goto tg_messeges_get_history_finish;
			}
			break;
			
		case id_messages_messagesSlice:
			{
				tl_messages_messagesSlice_t *msgs = 
					(tl_messages_messagesSlice_t *)tl;
				
				parse_msgs(
						tg, &c, 
						msgs->messages_len, 
						msgs->messages_, 
						data, 
						callback);

				goto tg_messeges_get_history_finish;
			}
			break;
			
		default:
			break;
	}
	

tg_messeges_get_history_thow_error:;
	// throw error
	char *err = tg_strerr(tl); 
	ON_ERR(tg, tl, "%s", err);
	free(err);

tg_messeges_get_history_finish:;
	// free tl
	/* TODO:  <29-11-24, yourname> */

	return c;
}
