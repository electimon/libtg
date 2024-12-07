#include "messages.h"
#include <stdint.h>
#include <string.h>
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


void tg_message_from_database(
		tg_t *tg, tg_message_t *m, uint32_t msg_id)
{
	if (!m){
		ON_ERR(tg, NULL, "TG_MESSAGE IS NULL");
		return;
	}
	memset(m, 0, sizeof(tg_message_t));

	struct str s;
	str_init(&s);
	str_appendf(&s, "SELECT ");
	
	#define TG_MESSAGE_ARG(t, n, type, name) \
		str_appendf(&s, name ", ");
	#define TG_MESSAGE_STR(t, n, type, name) \
		str_appendf(&s, name ", ");
	#define TG_MESSAGE_PER(t, n, type, name) \
		str_appendf(&s, name ", type_%s, ", name);
	TG_MESSAGE_ARGS
	#undef TG_MESSAGE_ARG
	#undef TG_MESSAGE_STR
	#undef TG_MESSAGE_PER
	
	str_appendf(&s, 
			"id FROM messages WHERE msg_id = %d AND id = %d;"
			, msg_id, tg->id);
		
	tg_sqlite3_for_each(tg, s.str, stmt){
		int col = 0;
		#define TG_MESSAGE_ARG(t, n, type, name) \
			m->n = sqlite3_column_int64(stmt, col++);
		#define TG_MESSAGE_STR(t, n, type, name) \
			if (sqlite3_column_bytes(stmt, col) > 0){ \
				m->n = strndup(\
					(char *)sqlite3_column_text(stmt, col),\
					sqlite3_column_bytes(stmt, col));\
				col++; \
			}
		#define TG_MESSAGE_PER(t, n, type, name) \
			m->n = sqlite3_column_int64(stmt, col); \
			m->type_##n = sqlite3_column_int64(stmt, col); \
			col++;
		
		TG_MESSAGE_ARGS
		#undef TG_MESSAGE_ARG
		#undef TG_MESSAGE_STR
		#undef TG_MESSAGE_PER
	}
}

void tg_message_from_tl(
		tg_t *tg, tg_message_t *tgm, tl_message_t *tlm)
{
	memset(tgm, 0, sizeof(tg_message_t));
	#define TG_MESSAGE_ARG(t, arg, ...) \
		tgm->arg = tlm->arg;
	#define TG_MESSAGE_STR(t, arg, ...) \
	if (tlm->arg.size > 0)\
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

	// handle with media
	if (tlm->media_){
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
			default:
				break;
		}
	}
}

static void parse_msg(
		int *c, tg_t *tg, tg_message_t *tgm, void *tlm_)
{
	printf("%s\n",__func__);
	tg_message_from_tl(tg, tgm, tlm_);
	*c += 1;
}

static void parse_msgs(
		tg_t *tg, int *c, 
		int argc, tl_t **argv,
		void *data,
		int (*callback)(void *data, 
			const tg_message_t *message))
{
	printf("%s\n",__func__);
	int i;
	for (i = 0; i < argc; ++i) {
		if (!argv[i] || argv[i]->_id != id_message)
			continue;
				
		tg_message_t m;
		parse_msg(c, tg, &m, argv[i]);
		if (callback)
			if (callback(data, &m))
				break;
	}
}	

int tg_messages_getHistory(
		tg_t *tg,
		tg_peer_t peer_,
		int offset_id,
		int offset_date,
		int add_offset,
		int limit,
		int max_id,
		int min_id,
		uint64_t *hash,
		void *data,
		int (*callback)(void *data, 
			const tg_message_t *message))
{
	tl_t *tl = NULL;

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

	tl = tg_send_query(tg, getHistory); 
	buf_free(getHistory);

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

int tg_send_message(tg_t *tg, tg_peer_t peer_, const char *message)
{
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

	tg_send_query(tg, m);
	buf_free(m);

	return 0;
}

struct _sync_messages_update_message_t{
	tg_t *tg;
	int d;
	uint64_t *hash;
	uint32_t peer_type;
	uint64_t peer_id;
	uint64_t peer_access_hash;
	void *userdata;
  void (*on_done)(void *userdata);
};

static int _sync_messages_update_message(
		void *data, 
		const tg_message_t *m)
{
	if (!m)
		return 0;

	struct _sync_messages_update_message_t *d = data;
	d->d = m->date_; 

	ON_LOG(d->tg, "%s: %d", __func__, m->date_);

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
	
	TG_MESSAGE_ARGS
	#undef TG_MESSAGE_ARG
	#undef TG_MESSAGE_STR
	#undef TG_MESSAGE_PER
	
	str_appendf(&s, "id = %d WHERE msg_id = %d;\n"
			, d->tg->id, m->id_);
	
	/*ON_LOG(d->tg, "%s: %s", __func__, s.str);*/
	if (tg_sqlite3_exec(d->tg, s.str) == 0){
		// update hash
		update_hash(d->hash, m->id_);
	}
	free(s.str);

	return 0;
}

static void _sync_messages_update(
		struct _sync_messages_update_message_t *d)
{
	uint64_t hash = 
		messages_hash_from_database(d->tg, d->peer_id);	 

	d->hash = &hash;

	tg_peer_t peer = 
	{d->peer_type, d->peer_id, d->peer_access_hash};
	
	tg_messages_getHistory(
			d->tg, 
			peer, 
			0, 
			d->d, 
			0, 
			10, 
			0, 
			0, 
			&hash, 
			d, 
			_sync_messages_update_message);
	
		messages_hash_to_database(
				d->tg, d->peer_id, hash);

	if (d->on_done)
		d->on_done(d->userdata);
}

static void * _sync_messages_thread(void * data)
{
	struct _sync_messages_update_message_t *d = data;
	ON_LOG(d->tg, "%s: start", __func__);

	ON_LOG(d->tg, "%s: updating messages...", __func__);	
	_sync_messages_update(d);

	free(d);

	pthread_exit(0);	
}

int tg_sync_messages_to_database(
		tg_t *tg,
		uint32_t date,
		tg_peer_t peer,
		void *userdata, void (*on_done)(void *userdata))
{
	// create table
	char sql[BUFSIZ] = 
		"CREATE TABLE IF NOT EXISTS messages (id INT, msg_id INT UNIQUE);";
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

	TG_MESSAGE_ARGS
	#undef TG_MESSAGE_ARG
	#undef TG_MESSAGE_STR
	#undef TG_MESSAGE_PER

	// set data
	struct _sync_messages_update_message_t *d = 
		NEW(struct _sync_messages_update_message_t, return 1);
	d->tg = tg;
	d->d  = date;
	d->userdata = userdata;
	d->on_done = on_done;
	d->peer_id = peer.id;
	d->peer_type = peer.type;
	d->peer_access_hash = peer.access_hash;
	
	//create new thread
	int err = pthread_create(
			&(tg->sync_dialogs_tid), 
			NULL, 
			_sync_messages_thread, 
			d);

	return err;
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
	TG_MESSAGE_ARGS
	#undef TG_MESSAGE_ARG
	#undef TG_MESSAGE_STR
	#undef TG_MESSAGE_PER
	
	str_appendf(&s, 
			"id FROM messages WHERE id = %d "
			"ORDER BY \'date\' DESC;", tg->id);
		
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
				col++; \
			}
		#define TG_MESSAGE_PER(t, n, type, name) \
			m.n = sqlite3_column_int64(stmt, col); \
			m.type_##n = sqlite3_column_int64(stmt, col); \
			col++;
		
		TG_MESSAGE_ARGS
		#undef TG_MESSAGE_ARG
		#undef TG_MESSAGE_STR
		#undef TG_MESSAGE_PER

		if (callback){
			if (callback(data, &m)){
				sqlite3_close(db);
				break;
			}
		}
	}	
	
	free(s.str);
	return 0;
}
