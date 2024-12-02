#ifndef TG_MESSAGES_H
#define TG_MESSAGES_H

#include "tg.h"

#define TG_MESSAGE_ARGS\
	TG_MESSAGE_ARG(bool, out, "INT", "out") \
	TG_MESSAGE_ARG(bool, mentioned, "INT", "mentioned") \
	TG_MESSAGE_ARG(bool, media_unread, "INT", "media_unread") \
	TG_MESSAGE_ARG(bool, silent, "INT", "silent") \
	TG_MESSAGE_ARG(bool, post, "INT", "post") \
	TG_MESSAGE_ARG(bool, from_scheduled, "INT", "from_scheduled") \
	TG_MESSAGE_ARG(bool, legacy, "INT", "legacy") \
	TG_MESSAGE_ARG(bool, edit_hide, "INT", "edit_hide") \
	TG_MESSAGE_ARG(bool, pinned, "INT", "pinned") \
	TG_MESSAGE_ARG(bool, noforwards, "INT", "noforwards") \
	TG_MESSAGE_ARG(bool, invert_media, "INT", "invert_media") \
	TG_MESSAGE_ARG(bool, offline, "INT", "offline") \
	TG_MESSAGE_ARG(int, id, "INT", "msg_id") \
	TG_MESSAGE_ARG(long, from_id, "INT", "from_id") \
	TG_MESSAGE_ARG(int, from_boosts_applied, "INT", "from_boosts_applied") \
	TG_MESSAGE_ARG(long, peer_id, "INT", "peer_id") \
	TG_MESSAGE_ARG(long, saved_peer_id, "INT", "saved_peer_id") \
	TG_MESSAGE_ARG(int, date, "INT", "date") \
	TG_MESSAGE_STR(char*, message, "TEXT", "message") \

typedef struct tg_message_t {
	#define TG_MESSAGE_ARG(type, name, ...) type name;
	#define TG_MESSAGE_STR(type, name, ...) type name;
	TG_MESSAGE_ARGS
	#undef TG_MESSAGE_ARG
	#undef TG_MESSAGE_STR
} tg_message_t;

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
