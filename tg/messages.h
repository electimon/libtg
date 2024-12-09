#ifndef TG_MESSAGES_H
#define TG_MESSAGES_H

#include "tg.h"
#include "peer.h"

#define TG_MESSAGE_ARGS\
	TG_MESSAGE_ARG(bool,     out_, "INT", "out") \
	TG_MESSAGE_ARG(bool,     mentioned_, "INT", "mentioned") \
	TG_MESSAGE_ARG(bool,     media_unread_, "INT", "media_unread") \
	TG_MESSAGE_ARG(bool,     silent_, "INT", "silent") \
	TG_MESSAGE_ARG(bool,     post_, "INT", "post") \
	TG_MESSAGE_ARG(bool,     from_scheduled_, "INT", "from_scheduled") \
	TG_MESSAGE_ARG(bool,     legacy_, "INT", "legacy") \
	TG_MESSAGE_ARG(bool,     edit_hide_, "INT", "edit_hide") \
	TG_MESSAGE_ARG(bool,     pinned_, "INT", "pinned") \
	TG_MESSAGE_ARG(bool,     noforwards_, "INT", "noforwards") \
	TG_MESSAGE_ARG(bool,     invert_media_, "INT", "invert_media") \
	TG_MESSAGE_ARG(bool,     offline_, "INT", "offline") \
	TG_MESSAGE_ARG(uint32_t, id_, "INT", "msg_id") \
	TG_MESSAGE_PER(uint64_t, from_id_, "INT", "from_id") \
	TG_MESSAGE_ARG(uint32_t, from_boosts_applied_, "INT", "from_boosts_applied") \
	TG_MESSAGE_PER(uint64_t, peer_id_, "INT", "peer_id") \
	TG_MESSAGE_PER(uint64_t, saved_peer_id_, "INT", "saved_peer_id") \
	TG_MESSAGE_ARG(uint32_t, date_, "INT", "date") \
	TG_MESSAGE_STR(char*,    message_, "TEXT", "message") \

typedef struct tg_message_t {
	#define TG_MESSAGE_ARG(t, arg, ...) t arg;
	#define TG_MESSAGE_STR(t, arg, ...) t arg;
	#define TG_MESSAGE_PER(t, arg, ...) t arg; int type_##arg; 
	TG_MESSAGE_ARGS
	#undef TG_MESSAGE_ARG
	#undef TG_MESSAGE_STR
	#undef TG_MESSAGE_PER
	uint64_t photo_id;
	uint64_t photo_access_hash;
	uint32_t photo_dc_id;
	uint32_t photo_date;
	char * photo_file_reference; 
} tg_message_t;

/* convert tl_message to tg_message */
void tg_message_from_tl(
		tg_t*, tg_message_t*, tl_message_t*);

/* get message from database */
void tg_message_from_database(
		tg_t*, tg_message_t*, uint32_t msg_id);

int tg_messages_getHistory(
		tg_t *tg,
		tg_peer_t peer,
		int offset_id,
		int offset_date,
		int add_offset,
		int limit,
		int max_id,
		int min_id,
		uint64_t *hash,
		void *data,
		int (*callback)(void *data, const tg_message_t *message));

void tg_sync_messages_to_database(
		tg_t *tg,
		uint32_t date,
		tg_peer_t peer,
		int count,
		void *userdata, void (*on_done)(void *userdata));

void tg_async_messages_to_database(
		tg_t *tg,
		uint32_t date,
		tg_peer_t peer,
		int count,
		void *userdata, void (*on_done)(void *userdata));


int tg_get_messages_from_database(tg_t *tg, tg_peer_t peer, void *data,
		int (*callback)(void *data, const tg_message_t *message));

int tg_send_message(tg_t *tg, tg_peer_t peer, const char *message);

#endif /* ifndef TG_MESSAGES_H */		
