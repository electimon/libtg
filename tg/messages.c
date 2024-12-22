#include "messages.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "peer.h"
#include "tg.h"
#include "database.h"
#include "../tl/alloc.h"

// #include <stdint.h>
#if INTPTR_MAX == INT32_MAX
    #define THIS_IS_32_BIT_ENVIRONMENT
		#define _LD_ "%lld"
#elif INTPTR_MAX == INT64_MAX
    #define THIS_IS_64_BIT_ENVIRONMENT
		#define _LD_ "%ld"
#else
    #error "Environment not 32 or 64-bit."
#endif

void tg_message_from_tl(
		tg_t *tg, tg_message_t *tgm, tl_message_t *tlm)
{
	ON_LOG(tg, "%s: start...", __func__);
	memset(tgm, 0, sizeof(tg_message_t));
	
	#define TG_MESSAGE_ARG(t, arg, ...) \
		tgm->arg = tlm->arg;
	#define TG_MESSAGE_STR(t, arg, ...) \
	if (tlm->arg.data && tlm->arg.size > 0)\
		tgm->arg = strndup((char*)tlm->arg.data, tlm->arg.size);
	#define TG_MESSAGE_PER(t, arg, ...) \
	if (tlm->arg){\
		tl_peerUser_t *peer = (tl_peerUser_t *)tlm->arg; \
		tgm->arg = peer->user_id_;\
		tgm->type_##arg = peer->_id;\
	}
	#define TG_MESSAGE_SPA(...)
	#define TG_MESSAGE_SPS(...)
	TG_MESSAGE_ARGS
	#undef TG_MESSAGE_ARG
	#undef TG_MESSAGE_STR
	#undef TG_MESSAGE_PER
	#undef TG_MESSAGE_SPA
	#undef TG_MESSAGE_SPS

	// handle with media
	if (tlm->media_){
		tgm->media_type = tlm->media_->_id;
		switch (tlm->media_->_id) {
			case id_messageMediaPhoto:
				{
					tl_messageMediaPhoto_t *mmp = 
						(tl_messageMediaPhoto_t *)tlm->media_;
					if (mmp->photo_ && mmp->photo_->_id == id_photo)
					{
						tl_photo_t *photo = (tl_photo_t *)mmp->photo_;
						tgm->photo_access_hash = photo->access_hash_; 
						tgm->photo_id = photo->id_;
						tgm->photo_dc_id = photo->dc_id_;
						tgm->photo_date = photo->date_;
						tgm->photo_file_reference = 
							buf_to_base64(photo->file_reference_);
					}
				}
				break;
			case id_messageMediaGeo:
				{
					tl_messageMediaGeo_t *mmp = 
						(tl_messageMediaGeo_t *)tlm->media_;
					if (mmp->geo_ && mmp->geo_->_id == id_geoPoint)
					{
						tl_geoPoint_t *geo       = (tl_geoPoint_t *)mmp->geo_;
						tgm->geo_long            = (uint64_t)geo->long_;
						tgm->geo_lat             = (uint64_t)geo->lat_;
						tgm->geo_access_hash     = geo->access_hash_;
						tgm->geo_accuracy_radius = geo->accuracy_radius_;
					}
				}
				break;
			case id_messageMediaDocument:
				{
					tl_messageMediaDocument_t *mmp = 
						(tl_messageMediaDocument_t *)tlm->media_;
					if (mmp->document_ && mmp->document_->_id == id_document)
					{
						tgm->doc_isVoice = mmp->video_;
						tgm->doc_isRound = mmp->round_;
						tgm->doc_isVoice = mmp->voice_;
						
						tl_document_t *doc = (tl_document_t *)mmp->document_;
						tgm->doc_id = doc->id_;
						tgm->doc_access_hash = doc->access_hash_;
						tgm->doc_file_reference =
							buf_to_base64(doc->file_reference_);
						tgm->doc_date = doc->date_;
						tgm->doc_size = doc->size_;
						tgm->doc_dc_id = doc->dc_id_;

						int i;
						// todo get thumbs
						for (i = 0; i < doc->thumbs_len; ++i) {
									
						}
						
						for (i = 0; i < doc->video_thumbs_len; ++i) {
							
						}

						// get attributes
						for (i = 0; i < doc->attributes_len; ++i) {
							tl_t *attr = doc->attributes_[i];
							if (!attr)
								continue;

							switch (attr->_id) {
								case id_documentAttributeImageSize:
									{
										tl_documentAttributeImageSize_t *a =
											(tl_documentAttributeImageSize_t *)attr;
										tgm->doc_w = a->w_;
										tgm->doc_h = a->h_;
									}
									break;
								case id_documentAttributeVideo:
									{
										tl_documentAttributeVideo_t *a =
											(tl_documentAttributeVideo_t *)attr;
										tgm->doc_vw = a->w_;
										tgm->doc_vh = a->h_;
										tgm->doc_vduration = a->duration_;
									}
									break;
								case id_documentAttributeAudio:
									{
										tl_documentAttributeAudio_t *a =
											(tl_documentAttributeAudio_t *)attr;
										tgm->doc_title =
											buf_to_base64(a->title_);
										tgm->doc_aduration = a->duration_;
									}
									break;
								case id_documentAttributeFilename:
									{
										tl_documentAttributeFilename_t *a =
											(tl_documentAttributeFilename_t *)attr;
										tgm->doc_file_name =
											buf_to_base64(a->file_name_);

									}
								
								default:
									break;	
							}
						}
					}
				}
				break;
			case id_messageMediaWebPage:
				{
					tl_messageMediaWebPage_t *mmp = 
						(tl_messageMediaWebPage_t *)tlm->media_;
					if (mmp->webpage_ && mmp->webpage_->_id == id_webPage)
					{
						tl_webPage_t *web = (tl_webPage_t *)mmp->webpage_;
						tgm->web_id = web->id_;
						tgm->web_url =
							buf_to_base64(web->url_);
						tgm->web_display_url =
							buf_to_base64(web->display_url_);
						tgm->web_hash = web->hash_;
						tgm->web_type =
							buf_to_base64(web->type_);
						tgm->web_site_name =
							buf_to_base64(web->site_name_);
						tgm->web_title =
							buf_to_base64(web->title_);
						tgm->web_description =
							buf_to_base64(web->description_);

						if (web->photo_ && web->photo_->_id == id_photo)
						{
							tl_photo_t *photo = (tl_photo_t *)web->photo_;
							tgm->web_photo_id = photo->id_;
							tgm->web_photo_access_hash = photo->access_hash_;
							tgm->web_photo_dc_id = photo->dc_id_;
							tgm->web_photo_date = photo->date_;
							tgm->web_photo_file_reference =
								buf_to_base64(photo->file_reference_);
						}
					}
				}
				break;
			case id_messageMediaContact:
				{					
					tl_messageMediaContact_t *mmp = 
						(tl_messageMediaContact_t *)tlm->media_;
					
					tgm->contact_phone_number = 
							buf_to_base64(mmp->phone_number_);
					tgm->contact_first_name = 
							buf_to_base64(mmp->first_name_);
					tgm->contact_last_name = 
							buf_to_base64(mmp->last_name_);
					tgm->contact_vcard = 
							buf_to_base64(mmp->vcard_);
					tgm->contact_user_id = mmp->user_id_;
				}
				break;
			
			default:
				break;
		}
	}
}

static void parse_msgs(
		tg_t *tg, uint64_t peer_id, 
		int argc, tl_t **argv,
		void *data,
		int (*callback)(void *, const tg_message_t*))
{
	int i;
	for (i = 0; i < argc; ++i) {
		if (!argv[i] || argv[i]->_id != id_message)
			continue;
				
		tg_message_t m;
		tg_message_from_tl(tg, &m, (tl_message_t *)argv[i]);
		
		// save message to database
		if (tg_message_to_database(tg, &m) == 0){
			// update hash
			uint64_t hash = 
				messages_hash_from_database(tg, peer_id);
			update_hash(&hash, m.id_);
			messages_hash_to_database(tg, peer_id, hash);
		}

		// callback
		if (callback)
			if (callback(data, &m))
				break;
	}
}	

struct tg_messages_get_history_t {
	tg_t *tg;
	uint64_t peer_id;
	void *userdata;
	int (*callback)(void *, const tg_message_t *);
	void (*on_done)(void *userdata);
};

static int tg_messages_get_history_cb(void *data, const tl_t *tl)
{
	struct tg_messages_get_history_t *s = data; 
	if (!tl){
		free(s);
		return 0;
	}
	ON_LOG(s->tg, "%s: recived id: %.8x", __func__, tl->_id);

	int i, k;

	switch (tl->_id) {
		case id_messages_channelMessages:
			{
				tl_messages_channelMessages_t *msgs = 
					(tl_messages_channelMessages_t *)tl;
				
				parse_msgs(
						s->tg, s->peer_id, 
						msgs->messages_len, 
						msgs->messages_, 
						s->userdata, 
						s->callback);

				goto tg_messeges_get_history_finish;
			}
			break;
			
		case id_messages_messages:
			{
				tl_messages_messages_t *msgs = 
					(tl_messages_messages_t *)tl;
				
				parse_msgs(
						s->tg, s->peer_id, 
						msgs->messages_len, 
						msgs->messages_, 
						s->userdata, 
						s->callback);

				goto tg_messeges_get_history_finish;
			}
			break;
			
		case id_messages_messagesSlice:
			{
				tl_messages_messagesSlice_t *msgs = 
					(tl_messages_messagesSlice_t *)tl;
				
				parse_msgs(
						s->tg, s->peer_id, 
						msgs->messages_len, 
						msgs->messages_, 
						s->userdata, 
						s->callback);

				goto tg_messeges_get_history_finish;
			}
			break;
			
		default:
			break;
	}
	
tg_messeges_get_history_finish:;
	if (s->on_done)
		s->on_done(s->userdata);
	free(s);
	return 0;
}

void tg_messages_get_history(
		tg_t *tg,
		tg_peer_t peer_,
		int offset_id,
		int offset_date,
		int add_offset,
		int limit,
		int max_id,
		int min_id,
		uint64_t *hash,
		void *userdata,
		int (*callback)(void *, const tg_message_t *),
		void (*on_done)(void *))
{
	int i, k, c = 0;
	uint64_t h = 0;
	if (hash)
		h = *hash;

	buf_t peer = tg_inputPeer(peer_); 

	buf_t getHistory = 
		tl_messages_getHistory(
				&peer, 
				offset_id, 
				offset_date, 
				add_offset, 
				limit, 
				max_id, 
				min_id, 
				h);
	buf_free(peer);

	struct tg_messages_get_history_t *s = 
		NEW(struct tg_messages_get_history_t, 
			ON_ERR(tg, "%s: can't allocate memory", __func__);
			return);
	s->tg = tg;
	s->peer_id = peer_.id;
	s->userdata = userdata;
	s->callback = callback;
	s->on_done = on_done;

	tg_queue_manager_send_query(
			tg, getHistory, 
			s, tg_messages_get_history_cb, 
			NULL, NULL);
}

struct tg_send_message_t {
	tg_t *tg;
	void *userdata; 
	void (*on_done)(void *userdata, bool out);
};

static int _tg_send_message_cb(void *data, const tl_t *tl)
{
	printf("%s\n", __func__);
	struct tg_send_message_t *s = data;
	if (!tl){
		if (s->on_done)
			s->on_done(s->userdata, false);
		free(s);
		return 0;
	}
	
	switch (tl->_id) {
		case id_updatesTooLong: case id_updateShortMessage:
		case id_updateShortChatMessage: 
			/* ???:  <16-12-24, yourname> */
			break;
		case id_updateShort:
			{
				tl_updateShort_t *us =
					(tl_updateShort_t *)tl;

				printf("UPDATE: %s (%.8x)\n",
						TL_NAME_FROM_ID(us->update_->_id), us->update_->_id);
			
				if (s->on_done)
					s->on_done(s->userdata, true);
			}
			break;
		case id_updates:
			{
				tl_updates_t *us =
					(tl_updates_t *)tl;
				int i;
				for (i = 0; i < us->updates_len; ++i) {
					printf("UPDATE: %s (%.8x)\n",
							TL_NAME_FROM_ID(us->updates_[i]->_id), 
							us->updates_[i]->_id);
				}
				if (s->on_done)
					s->on_done(s->userdata, true);
			}
			break;
		case id_updateShortSentMessage:
			{
				tl_updateShortSentMessage_t *usm =
					(tl_updateShortSentMessage_t *)tl;
				if (usm->out_){
					if (s->on_done)
						s->on_done(s->userdata, true);
				} else {
					if (s->on_done)
						s->on_done(s->userdata, false);
				}
			}	
			break;

		default:
			if (s->on_done)
				s->on_done(s->userdata, false);
			break;
	}	

	free(s);
	return 0;
}

void tg_send_message(tg_t *tg, tg_peer_t peer_,
		const char *message, void *userdata, 
		void (*on_done)(void *userdata, bool out))
{
	printf("TO SEND MESSAGE: %s\n", message);
	buf_t peer = tg_inputPeer(peer_); 
	buf_t random_id = buf_rand(8);
	buf_t m = tl_messages_sendMessage(
			NULL, 
			NULL, 
			NULL, 
			NULL, 
			NULL, 
			NULL, 
			NULL, 
			&peer, 
			NULL, 
			message, 
			buf_get_ui64(random_id), 
			NULL, 
			NULL, 
			0, 
			NULL, 
			NULL, 
			NULL, 
			NULL);
	buf_free(peer);
	buf_free(random_id);

	struct tg_send_message_t *s = NEW(struct tg_send_message_t, 
			ON_ERR(tg, "%s: can't allocate memory", __func__);
			return);
	s->tg = tg;
	s->userdata = userdata;
	s->on_done = on_done;

	tg_queue_manager_send_query(
			tg, m, 
			s, _tg_send_message_cb, 
			NULL, NULL);
}

int tg_message_to_database(tg_t *tg, const tg_message_t *m)
{
	// save message to database
	struct str s;
	str_init(&s);

	str_appendf(&s,
		"INSERT INTO \'messages\' (\'msg_id\') "
		"SELECT %d "
		"WHERE NOT EXISTS (SELECT 1 FROM messages WHERE msg_id = %d);\n"
		, m->id_, m->id_);

	str_appendf(&s, "UPDATE \'messages\' SET ");
	
	#define TG_MESSAGE_STR(t, n, type, name) \
	if (m->n){\
		str_appendf(&s, "\'" name "\'" " = \'"); \
		str_append(&s, (char*)m->n, strlen((char*)m->n)); \
		str_appendf(&s, "\', "); \
	}
		
	#define TG_MESSAGE_ARG(t, n, type, name) \
		str_appendf(&s, "\'" name "\'" " = "_LD_", ", (uint64_t)m->n);
	
	#define TG_MESSAGE_PER(t, n, type, name) \
		str_appendf(&s, "\'" name "\'" " = "_LD_", ", (uint64_t)m->n); \
		str_appendf(&s, "\'type_%s\' = "_LD_", ", name, (uint64_t)m->type_##n);
	
	#define TG_MESSAGE_SPA(t, n, type, name) \
		str_appendf(&s, "\'" name "\'" " = "_LD_", ", (uint64_t)m->n);
	
	#define TG_MESSAGE_SPS(t, n, type, name) \
	if (m->n){\
		str_appendf(&s, "\'" name "\'" " = \'"); \
		str_append(&s, (char*)m->n, strlen((char*)m->n)); \
		str_appendf(&s, "\', "); \
	}

	TG_MESSAGE_ARGS
	#undef TG_MESSAGE_ARG
	#undef TG_MESSAGE_STR
	#undef TG_MESSAGE_PER
	#undef TG_MESSAGE_SPA
	#undef TG_MESSAGE_SPS

	str_appendf(&s, "id = %d WHERE msg_id = %d;\n"
			, tg->id, m->id_);
	
	/*ON_LOG(d->tg, "%s: %s", __func__, s.str);*/
	int ret = tg_sqlite3_exec(tg, s.str);
	
	free(s.str);
	
	return ret;
}

void tg_messages_create_table(tg_t *tg){
	char sql[BUFSIZ]; 
	
	sprintf(sql,
		"CREATE TABLE IF NOT EXISTS messages (id INT, msg_id INT UNIQUE); ");
	ON_LOG(tg, "%s", sql);
	tg_sqlite3_exec(tg, sql);	
	
	#define TG_MESSAGE_ARG(t, n, type, name) \
		sprintf(sql, "ALTER TABLE \'messages\' ADD COLUMN "\
				"\'" name "\' " type ";");\
		ON_LOG(tg, "%s", sql);\
		tg_sqlite3_exec(tg, sql);	
	#define TG_MESSAGE_STR(t, n, type, name) \
		sprintf(sql, "ALTER TABLE \'messages\' ADD COLUMN "\
				"\'" name "\' " type ";");\
		ON_LOG(tg, "%s", sql);\
		tg_sqlite3_exec(tg, sql);	
	#define TG_MESSAGE_PER(t, n, type, name) \
		sprintf(sql, "ALTER TABLE \'messages\' ADD COLUMN "\
				"\'" name "\' " type ";");\
		ON_LOG(tg, "%s", sql);\
		tg_sqlite3_exec(tg, sql);	\
		sprintf(sql, "ALTER TABLE \'messages\' ADD COLUMN "\
				"\'type_%s\' " type ";", name);\
		ON_LOG(tg, "%s", sql);\
		tg_sqlite3_exec(tg, sql);	
	#define TG_MESSAGE_SPA(t, n, type, name) \
		sprintf(sql, "ALTER TABLE \'messages\' ADD COLUMN "\
				"\'" name "\' " type ";");\
		ON_LOG(tg, "%s", sql);\
		tg_sqlite3_exec(tg, sql);
	#define TG_MESSAGE_SPS(t, n, type, name) \
		sprintf(sql, "ALTER TABLE \'messages\' ADD COLUMN "\
				"\'" name "\' " type ";");\
		ON_LOG(tg, "%s", sql);\
		tg_sqlite3_exec(tg, sql);
	TG_MESSAGE_ARGS
	#undef TG_MESSAGE_ARG
	#undef TG_MESSAGE_STR
	#undef TG_MESSAGE_PER
	#undef TG_MESSAGE_SPA
	#undef TG_MESSAGE_SPS
} 

int tg_get_messages_from_database(tg_t *tg, tg_peer_t peer, void *data,
		int (*callback)(void *data, const tg_message_t *message))
{
	struct str s;
	str_init(&s);
	str_appendf(&s, "SELECT ");
	
	#define TG_MESSAGE_ARG(t, n, type, name) \
		str_appendf(&s, name ", ");
	#define TG_MESSAGE_STR(t, n, type, name) \
		str_appendf(&s, name ", ");
	#define TG_MESSAGE_PER(t, n, type, name) \
		str_appendf(&s, name ", type_%s, ", name);
	#define TG_MESSAGE_SPA(t, n, type, name) \
		str_appendf(&s, name ", ");
	#define TG_MESSAGE_SPS(t, n, type, name) \
		str_appendf(&s, name ", ");
	TG_MESSAGE_ARGS
	#undef TG_MESSAGE_ARG
	#undef TG_MESSAGE_STR
	#undef TG_MESSAGE_PER
	#undef TG_MESSAGE_SPA
	#undef TG_MESSAGE_SPS
		
	str_appendf(&s, 
			"id FROM messages WHERE id = %d AND peer_id = "_LD_" "
			"ORDER BY \'date\' DESC;", tg->id, peer.id);

	int i = 0;
	tg_sqlite3_for_each(tg, s.str, stmt){
		tg_message_t m;
		memset(&m, 0, sizeof(m));
		
		int col = 0;
		#define TG_MESSAGE_ARG(t, n, type, name) \
			m.n = sqlite3_column_int64(stmt, col++);
		#define TG_MESSAGE_STR(t, n, type, name) \
			if (sqlite3_column_bytes(stmt, col) > 0){ \
				m.n = strndup(\
					(char *)sqlite3_column_text(stmt, col),\
					sqlite3_column_bytes(stmt, col));\
			}\
			col++;
		#define TG_MESSAGE_PER(t, n, type, name) \
			m.n = sqlite3_column_int64(stmt, col); \
			m.type_##n = sqlite3_column_int64(stmt, col); \
			col++; col++;
		#define TG_MESSAGE_SPA(t, n, type, name) \
			m.n = sqlite3_column_int64(stmt, col++);
		#define TG_MESSAGE_SPS(t, n, type, name) \
			if (sqlite3_column_bytes(stmt, col) > 0){ \
				m.n = strndup(\
					(char *)sqlite3_column_text(stmt, col),\
					sqlite3_column_bytes(stmt, col));\
			}\
			col++;
		TG_MESSAGE_ARGS

		#undef TG_MESSAGE_ARG
		#undef TG_MESSAGE_STR
		#undef TG_MESSAGE_PER
		#undef TG_MESSAGE_SPA
		#undef TG_MESSAGE_SPS

		i++;
		
		if (callback){
			int ret = callback(data, &m);
			if (ret){
				sqlite3_close(db);
				return i;
			}
		}
	}	
	
	free(s.str);
	return i;
}

struct tg_messages_set_typing_t {
	tg_t *tg;
	void *userdata;
	void (*on_done)(void *userdata, bool ack);
};

static int tg_messages_set_typing_cb(void *data, const tl_t *tl)
{
	struct tg_messages_set_typing_t *s = data;
	if (!tl){
		if (s->on_done)
			s->on_done(s->userdata, false);
		free(s);
		return 0;
	}

	if (s->on_done)
		s->on_done(s->userdata, tl->_id == id_true?true:false);

	free(s);
	return 0;
}

void tg_messages_set_typing(tg_t *tg, tg_peer_t peer_,
		bool typing, void *userdata, 
		void (*on_done)(void *userdata, bool ack))
{
	Peer peer = tg_inputPeer(peer_); 
	SendMessageAction action;
	if (typing){
		action = tl_sendMessageTypingAction();
	} else {
		action = tl_sendMessageCancelAction();
	}

	buf_t setTyping = tl_messages_setTyping(
			&peer,
			NULL,
			&action);
	buf_free(peer);
	buf_free(action);

	struct tg_messages_set_typing_t *s = NEW(
			struct tg_messages_set_typing_t, 
			ON_ERR(tg, "%s: can't allocate memory", __func__);
			return;);

	tg_queue_manager_send_query(
			tg, 
			setTyping, 
			s, tg_messages_set_typing_cb, 
			NULL, NULL);
}

struct tg_messages_set_read_t {
	tg_t *tg;
	void *userdata;
	void (*on_done)(void *userdata);
};

static int tg_messages_set_read_cb(void *data, const tl_t *tl)
{
	struct tg_messages_set_read_t *s = data;
	if (!tl){
		if (s->on_done)
			s->on_done(s->userdata);
		free(s);
		return 0;
	}

	/* TODO: messages.AffectedMessages */
	if (s->on_done)
		s->on_done(s->userdata);

	free(s);
	return 0;
}

void tg_messages_set_read(tg_t *tg, tg_peer_t peer_,
		uint32_t max_id, void *userdata, 
		void (*on_done)(void *userdata))
{
	Peer peer = tg_inputPeer(peer_); 

	buf_t readHistory = tl_messages_readHistory(
			&peer, max_id);
	buf_free(peer);

	struct tg_messages_set_typing_t *s = NEW(
			struct tg_messages_set_typing_t, 
			ON_ERR(tg, "%s: can't allocate memory", __func__);
			return;);

	tg_queue_manager_send_query(
			tg, 
			readHistory, 
			s, tg_messages_set_typing_cb, 
			NULL, NULL);
}
