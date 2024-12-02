#ifndef TG_MESSAGES_H
#define TG_MESSAGES_H

#include "tg.h"

#define TG_MESSAGE_ARGS\
	TG_MESSAGE_ARG(bool, out_, "INT", "out") \
	TG_MESSAGE_ARG(bool, mentioned_, "INT", "mentioned") \
	TG_MESSAGE_ARG(bool, media_unread_, "INT", "media_unread") \
	TG_MESSAGE_ARG(bool, silent_, "INT", "silent") \
	TG_MESSAGE_ARG(bool, post_, "INT", "post") \
	TG_MESSAGE_ARG(bool, from_scheduled_, "INT", "from_scheduled") \
	TG_MESSAGE_ARG(bool, legacy_, "INT", "legacy") \
	TG_MESSAGE_ARG(bool, edit_hide_, "INT", "edit_hide") \
	TG_MESSAGE_ARG(bool, pinned_, "INT", "pinned") \
	TG_MESSAGE_ARG(bool, noforwards_, "INT", "noforwards") \
	TG_MESSAGE_ARG(bool, invert_media_, "INT", "invert_media") \
	TG_MESSAGE_ARG(bool, offline_, "INT", "offline") \
	TG_MESSAGE_ARG(int,  id_, "INT", "msg_id") \
	TG_MESSAGE_PER(long, from_id_, "INT", "from_id") \
	TG_MESSAGE_ARG(int,  from_boosts_applied_, "INT", "from_boosts_applied") \
	TG_MESSAGE_PER(long, peer_id_, "INT", "peer_id") \
	TG_MESSAGE_PER(long,  saved_peer_id_, "INT", "saved_peer_id") \
	TG_MESSAGE_ARG(int,   date_, "INT", "date") \
	TG_MESSAGE_STR(char*, message_, "TEXT", "message") \

typedef struct tg_message_t {
	#define TG_MESSAGE_ARG(t, arg, ...) t arg;
	#define TG_MESSAGE_STR(t, arg, ...) t arg;
	#define TG_MESSAGE_PER(t, arg, ...) t arg; int type_##arg; 
	TG_MESSAGE_ARGS
	#undef TG_MESSAGE_ARG
	#undef TG_MESSAGE_STR
	#undef TG_MESSAGE_PER
} tg_message_t;

/* convert tl_message to tg_message */
void tg_message_from_tl(tg_message_t*, tl_message_t*);

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
		int (*callback)(void *data, const tg_message_t *message));

int tg_async_messages_to_database(tg_t *tg, buf_t peer, int seconds);
int tg_get_messages_from_database(tg_t *tg, buf_t peer, void *data,
		int (*callback)(void *data, const tg_message_t *message));

#endif /* ifndef TG_MESSAGES_H */		
