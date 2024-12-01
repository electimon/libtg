#ifndef TG_DIALOGS_H_
#define TG_DIALOGS_H_

#include "tg.h"

#define TG_DIALOG_ARGS\
	TG_DIALOG_STR(char*, name, "TEXT", "name") \
	TG_DIALOG_ARG(bool, pinned, "INT", "pinned") \
	TG_DIALOG_ARG(bool, unread_mark, "INT", "unread_mark") \
	TG_DIALOG_ARG(int, top_message_id, "INT", "top_message_id") \
	TG_DIALOG_ARG(int, top_message_from_peer_type, "INT", "top_message_from_peer_type") \
	TG_DIALOG_ARG(long, top_message_from_peer_id, "INT", "top_message_from_peer_id") \
	TG_DIALOG_ARG(int, top_message_date, "INT", "top_message_date") \
	TG_DIALOG_STR(char*, top_message_text, "TEXT", "top_message_text") \
	TG_DIALOG_ARG(int, read_inbox_max_id, "INT", "read_inbox_max_id") \
	TG_DIALOG_ARG(int, read_outbox_max_id, "INT", "read_outbox_max_id") \
	TG_DIALOG_ARG(int, unread_count, "INT", "unread_count") \
	TG_DIALOG_ARG(int, unread_mentions_count, "INT", "unread_mentions_count") \
	TG_DIALOG_ARG(int, unread_reactions_count, "INT", "unread_reactions_count") \
	TG_DIALOG_ARG(int, folder_id, "INT", "folder_id") \
	TG_DIALOG_ARG(bool, silent, "INT", "silent") \
	TG_DIALOG_ARG(int, mute_until, "INT", "mute_until") \
	TG_DIALOG_ARG(int, peer_type, "INT", "peer_type") \
	TG_DIALOG_ARG(long, peer_id, "INT", "peer_id") \
	TG_DIALOG_ARG(long, photo_id, "INT", "photo_id") \
	TG_DIALOG_STR(char*, thumb, "TEXT", "thumb") \


typedef enum {
	TG_PEER_TYPE_NULL = 0,
	TG_PEER_TYPE_USER = id_peerUser,
	TG_PEER_TYPE_CHANNEL = id_peerChannel,
	TG_PEER_TYPE_CHAT = id_peerChat,
} TG_PEER_TYPE;

typedef struct tg_dialog_ {
	#define TG_DIALOG_ARG(type, name, ...) type name;
	#define TG_DIALOG_STR(type, name, ...) type name;
	TG_DIALOG_ARGS
	#undef TG_DIALOG_ARG
	#undef TG_DIALOG_STR
} tg_dialog_t;

/* get %limit number of dialogs older then %date and
 * %top_msg_id, callback dialogs array and update messages
 * %hash (if not NULL) 
 * set folder_id NULL to get all folders, pointer to 0 for 
 * non-hidden dialogs, pointer to 1 for hidden dialogs 
 * return number of dialogs*/ 
int tg_get_dialogs(
		tg_t *tg, 
		int limit,
		time_t date, 
		long * hash, 
		int *folder_id, 
		void *data,
		int (*callback)(void *data, 
			const tg_dialog_t *dialog));

int tg_async_dialogs_to_database(tg_t *tg, int seconds);
int tg_get_dialogs_from_database(tg_t *tg, void *data,
		int (*callback)(void *data, const tg_dialog_t *dialog));

#endif /* ifndef TG_DIALOGS_H_ */
