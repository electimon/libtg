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
	TG_MESSAGE_SPA(uint32_t, media_type, "INT", "media_type") \
	TG_MESSAGE_SPA(uint64_t, photo_id, "INT", "photo_id") \
	TG_MESSAGE_SPA(uint64_t, photo_access_hash, "INT", "photo_access_hash") \
	TG_MESSAGE_SPA(uint32_t, photo_dc_id, "INT", "photo_dc_id") \
	TG_MESSAGE_SPA(uint32_t, photo_date, "INT", "photo_date") \
	TG_MESSAGE_SPS(char*,    photo_file_reference, "TEXT", "photo_file_reference") \
	TG_MESSAGE_SPA(uint64_t, geo_long, "INT", "geo_long") \
	TG_MESSAGE_SPA(uint64_t, geo_lat, "INT", "geo_lat") \
	TG_MESSAGE_SPA(uint64_t, geo_access_hash, "INT", "geo_access_hash") \
	TG_MESSAGE_SPA(uint32_t, geo_accuracy_radius, "INT", "geo_accuracy_radius") \
	TG_MESSAGE_SPA(bool,     doc_isVideo, "INT", "doc_isVideo") \
	TG_MESSAGE_SPA(bool,     doc_isRound, "INT", "doc_isRound") \
	TG_MESSAGE_SPA(bool,     doc_isVoice, "INT", "doc_isVoice") \
	TG_MESSAGE_SPA(uint64_t, doc_id, "INT", "doc_id") \
	TG_MESSAGE_SPA(uint64_t, doc_access_hash, "INT", "doc_access_hash") \
	TG_MESSAGE_SPS(char*,    doc_file_reference, "TEXT", "doc_file_reference") \
	TG_MESSAGE_SPA(uint32_t, doc_date, "INT", "doc_date") \
	TG_MESSAGE_SPS(char*,    doc_mime_type, "TEXT", "doc_mime_type") \
	TG_MESSAGE_SPA(uint64_t, doc_size, "INT", "doc_size") \
	TG_MESSAGE_SPS(char*,    doc_thumbs, "TEXT", "doc_thumbs") \
	TG_MESSAGE_SPS(char*,    doc_video_thumbs, "TEXT", "doc_video_thumbs") \
	TG_MESSAGE_SPA(uint32_t, doc_dc_id, "INT", "doc_dc_id") \
	TG_MESSAGE_SPS(char*,    doc_file_name, "TEXT", "doc_file_name") \
	TG_MESSAGE_SPA(uint32_t, doc_w, "INT", "doc_w") \
	TG_MESSAGE_SPA(uint32_t, doc_h, "INT", "doc_h") \
	TG_MESSAGE_SPA(uint32_t, doc_vw, "INT", "doc_vw") \
	TG_MESSAGE_SPA(uint32_t, doc_vh, "INT", "doc_vh") \
	TG_MESSAGE_SPA(uint64_t, doc_aduration, "INT", "doc_aduration") \
	TG_MESSAGE_SPA(uint64_t, doc_vduration, "INT", "doc_vduration") \
	TG_MESSAGE_SPS(char*,    doc_title, "TEXT", "doc_title") \
	TG_MESSAGE_SPA(uint64_t, web_id, "INT", "web_id") \
	TG_MESSAGE_SPS(char*,    web_url, "TEXT", "web_url") \
	TG_MESSAGE_SPS(char*,    web_display_url, "TEXT", "web_display_url") \
	TG_MESSAGE_SPA(uint32_t, web_hash, "INT", "web_hash") \
	TG_MESSAGE_SPS(char*,    web_type, "TEXT", "web_type") \
	TG_MESSAGE_SPS(char*,    web_site_name, "TEXT", "web_site_name") \
	TG_MESSAGE_SPS(char*,    web_title, "TEXT", "web_title") \
	TG_MESSAGE_SPS(char*,    web_description, "TEXT", "web_description") \
	TG_MESSAGE_SPA(uint64_t, web_photo_id, "INT", "web_photo_id") \
	TG_MESSAGE_SPA(uint64_t, web_photo_access_hash, "INT", "web_photo_access_hash") \
	TG_MESSAGE_SPA(uint32_t, web_photo_dc_id, "INT", "web_photo_dc_id") \
	TG_MESSAGE_SPA(uint32_t, web_photo_date, "INT", "web_photo_date") \
	TG_MESSAGE_SPS(char*,    web_photo_file_reference, "TEXT", "web_photo_file_reference") \
	TG_MESSAGE_SPS(char*,    contact_phone_number, "TEXT", "contact_phone_number") \
	TG_MESSAGE_SPS(char*,    contact_first_name, "TEXT", "contact_first_name") \
	TG_MESSAGE_SPS(char*,    contact_last_name, "TEXT", "contact_last_name") \
	TG_MESSAGE_SPS(char*,    contact_vcard, "TEXT", "contact_vcard") \
	TG_MESSAGE_SPA(uint64_t, contact_user_id, "INT", "contact_user_id") \


typedef struct tg_message_ {
	#define TG_MESSAGE_ARG(t, arg, ...) t arg;
	#define TG_MESSAGE_STR(t, arg, ...) t arg;
	#define TG_MESSAGE_PER(t, arg, ...) t arg; int type_##arg; 
	#define TG_MESSAGE_SPA(t, arg, ...) t arg;
	#define TG_MESSAGE_SPS(t, arg, ...) t arg;
	TG_MESSAGE_ARGS
	#undef TG_MESSAGE_ARG
	#undef TG_MESSAGE_STR
	#undef TG_MESSAGE_PER
	#undef TG_MESSAGE_SPA
	#undef TG_MESSAGE_SPS
} tg_message_t;

/* convert tl_message to tg_message */
void tg_message_from_tl(
		tg_t*, tg_message_t*, tl_message_t*);

/* get message from database */
void tg_message_from_database(
		tg_t*, tg_message_t*, uint32_t msg_id);

void tg_messages_create_table(tg_t *tg);
int tg_message_to_database(tg_t *tg, const tg_message_t *m);

tg_message_t *tg_message_get(tg_t *tg, uint32_t msg_id);

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
		tg_peer_t peer,
		int offset,
		int limit,
		void *userdata, void (*on_done)(void *userdata));

void tg_async_messages_to_database(
		tg_t *tg,
		tg_peer_t peer,
		int offset,
		int limit,
		void *userdata, void (*on_done)(void *userdata));


int tg_get_messages_from_database(tg_t *tg, tg_peer_t peer, void *data,
		int (*callback)(void *data, const tg_message_t *message));

int tg_send_message(tg_t *tg, tg_peer_t peer, const char *message);

#endif /* ifndef TG_MESSAGES_H */		
