/**
 * File              : dialogs.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 29.11.2024
 * Last Modified Date: 12.12.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#include "tg.h"
#include "../tl/id.h"
#include "../mtx/include/net.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "time.h"
#include "../tl/alloc.h"
#include "dialogs.h"
#include "pthread.h"
#include "database.h"
#include <unistd.h>
#include "../transport/net.h"
#include <string.h>
#include "peer.h"

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


#define BUF2STR(_b) strndup((char*)_b.data, _b.size)
#define BUF2IMG(_b) \
	({buf_t i = image_from_photo_stripped(_b); \
	 buf_to_base64(i);}) 

int tg_get_dialogs(
		tg_t *tg, 
		int limit, 
		time_t date, 
		uint64_t * hash, 
		uint32_t *folder_id, 
		void *data,
		int (*callback)(void *data, 
			const tg_dialog_t *dialog))
{
	int i, k;
	uint64_t h = 0;
	if (hash)
		h = *hash;

	//InputPeer inputPeer = tl_inputPeerSelf();
	InputPeer inputPeer = tl_inputPeerEmpty();

	buf_t getDialogs = 
		tl_messages_getDialogs(
				NULL,
				folder_id, 
				date,
				-1, 
				&inputPeer, 
				limit,
				h);

	tl_t *tl = tg_send_query_to_net(
			tg, getDialogs, 
			true, tg->sync_dialogs_sockfd);
	if (tl && tl->_id == id_messages_dialogsNotModified){
		ON_LOG(tg, "%s: dialogs not modified", __func__);
		tl_messages_dialogsNotModified_t *dnm =
			(tl_messages_dialogsNotModified_t *)tl;
		return dnm->count_;
	}

	if ((tl && tl->_id == id_messages_dialogsSlice) ||
	    (tl && tl->_id == id_messages_dialogs))
	{
		tl_messages_dialogs_t md;

		if ((tl && tl->_id == id_messages_dialogsSlice))
		{
			tl_messages_dialogsSlice_t *mds = 
				(tl_messages_dialogsSlice_t *)tl;
			md.dialogs_ = mds->dialogs_; 
			md.dialogs_len = mds->dialogs_len; 
			md.chats_ = mds->chats_;
			md.chats_len = mds->chats_len;
			md.messages_ = mds->messages_;
			md.messages_len = mds->messages_len;
			md.users_ = mds->users_;
			md.users_len = mds->users_len;
		}

		if (tl && tl->_id == id_messages_dialogs)
		{
			md = *(tl_messages_dialogs_t *)tl;
		}
		
		ON_LOG(tg, "%s: got %d dialogs", 
				__func__, md.dialogs_len);

		for (i = 0; i < md.dialogs_len; ++i) {
			// handle dialogs
			tg_dialog_t d;
			memset(&d, 0, sizeof(d));

			tl_dialog_t dialog;
			memset(&d, 0, sizeof(tl_dialog_t));
			
			if (md.dialogs_[i]->_id == id_dialogFolder){
				tl_dialogFolder_t *df = 
					(tl_dialogFolder_t *)md.dialogs_[i];
				tl_folder_t *folder = 
					(tl_folder_t *)df->folder_;

				dialog.peer_ = df->peer_;
				dialog.top_message_ = df->top_message_;
				dialog.pinned_ = df->pinned_;

			} else if (md.dialogs_[i]->_id != id_dialog){
				ON_LOG(tg, "%s: unknown dialog type: %.8x",
						__func__, md.dialogs_[i]->_id);
				continue;
			}
			dialog = *(tl_dialog_t *)md.dialogs_[i];	

			d.top_message_id = dialog.top_message_;
			
			d.pinned = dialog.pinned_;
			d.unread_mark = dialog.unread_mark_;
			d.read_inbox_max_id = dialog.read_inbox_max_id_;
			d.read_outbox_max_id = dialog.read_outbox_max_id_;
			d.unread_count = dialog.unread_count_;
			d.unread_mentions_count = dialog.unread_mentions_count_;
			d.unread_reactions_count = dialog.unread_reactions_count_;
			d.folder_id = dialog.folder_id_;
			
			tl_peerChat_t *peer = (tl_peerChat_t *)dialog.peer_;
			if (peer){
				d.peer_id = peer->chat_id_;
			}

			tl_peerNotifySettings_t *pns = 
				(tl_peerNotifySettings_t *)dialog.notify_settings_;
			if (pns){
				d.silent = pns->silent_;
				d.mute_until = pns->mute_until_;
			}

			int k;

			// iterate users
			for (k = 0; k < md.users_len; ++k) {
				// skip on NULL
				if (!md.users_[k])
					continue;

				switch (md.users_[k]->_id) {
					case id_user:
						{
							tl_user_t *user = 
								(tl_user_t *)md.users_[k];
							
							if (d.peer_id == user->id_)
							{
								d.access_hash = user->access_hash_;
								d.peer_type = TG_PEER_TYPE_USER;
								if (user->username_.size)
									d.name = BUF2STR(user->username_);
								else
									d.name = BUF2STR(user->first_name_);
							
								if (user->photo_ && 
										user->photo_->_id == id_userProfilePhoto)
								{
									tl_userProfilePhoto_t *photo = 
										(tl_userProfilePhoto_t *)user->photo_; 
									d.photo_id = photo->photo_id_;
									d.thumb = BUF2IMG(photo->stripped_thumb_);
								}
							}
						}
						break;
				  case id_userEmpty:
						{
							tl_userEmpty_t *user =
								(tl_userEmpty_t *)md.users_[k];
							
							if (d.peer_id == user->id_)
							{
								d.peer_type = TG_PEER_TYPE_USER;
								d.name = strdup("empty"); 
							}
						}
						break;
					
					default:
						break;
				}
			}

			// iterate chats
			for (k = 0; k < md.chats_len; ++k) {
				// skip on NULL
				if (!md.chats_[k])
					continue;
				
				switch (md.chats_[k]->_id) {
				  case id_channel:
						{
							tl_channel_t *channel = 
								(tl_channel_t *)md.chats_[k];
							if (d.peer_id == channel->id_)
							{
								d.access_hash = channel->access_hash_;
								d.peer_type = TG_PEER_TYPE_CHANNEL;
								d.name = BUF2STR(channel->title_);
								if (channel->photo_ && 
									channel->photo_->_id == id_chatPhoto)
								{
									tl_chatPhoto_t *photo = 
										(tl_chatPhoto_t *)channel->photo_; 
									d.photo_id = photo->photo_id_;
									d.thumb = BUF2IMG(photo->stripped_thumb_);
								}
							}
						}
						break;
				  
					case id_channelForbidden:
						{
							tl_channelForbidden_t *channel = 
								(tl_channelForbidden_t *)md.chats_[k];
							if (d.peer_id == channel->id_)
							{
								d.peer_type = TG_PEER_TYPE_CHANNEL;
								d.name = BUF2STR(channel->title_);
							}
						}
						break;
					
					case id_chat:
						{
							tl_chat_t *chat = 
								(tl_chat_t *)md.chats_[k];
							if (d.peer_id == chat->id_)
							{
								d.peer_type = TG_PEER_TYPE_CHAT;
								d.name = BUF2STR(chat->title_);
								if (chat->photo_ && 
									chat->photo_->_id == id_chatPhoto)
								{
									tl_chatPhoto_t *photo = 
										(tl_chatPhoto_t *)chat->photo_; 
									d.photo_id = photo->photo_id_;
									d.thumb = BUF2IMG(photo->stripped_thumb_);
								}
							}
						}
						break;
				  
					case id_chatForbidden:
						{
							tl_chatForbidden_t *chat = 
								(tl_chatForbidden_t *)md.chats_[k];
							if (d.peer_id == chat->id_){
								d.peer_type = TG_PEER_TYPE_CHAT;
								d.name = BUF2STR(chat->title_);
							}
						}
						break;
					

					default:
						break;
				}
			}
				
			// iterate messages
			for (k = 0; k < md.messages_len; ++k){
				if (md.messages_[k] && 
						md.messages_[k]->_id == id_message)
				{
					tl_message_t *message = 
						(tl_message_t *)md.messages_[k];
					if (message->id_ == d.top_message_id){
						d.top_message_date = message->date_;
						d.top_message_text = BUF2STR(message->message_);
						/*
						if (message->peer_id_){
							tl_peerUser_t *from = 
								(tl_peerUser_t *)message->from_id_;
							switch (message->peer_id_->_id) {
								case id_peerUser:
									d.top_message_from_peer_type = TG_PEER_TYPE_USER;	
									d.top_message_from_peer_id = from->user_id_;
									break;
								case id_peerChannel:
									tl_peerChannel_t *from = 
										(tl_peerChannel_t *)message->from_id_;
									d.top_message_from_peer_type = TG_PEER_TYPE_CHANNEL;	
									d.top_message_from_peer_id = from->user_id_;
									break;
								case id_peerChat:
									d.top_message_from_peer_type = TG_PEER_TYPE_CHAT;	
									d.top_message_from_peer_id = from->user_id_;
									break;
								
								default:
									d.top_message_from_peer_type = TG_PEER_TYPE_NULL;	
									break;
									
							}
						}
						*/
					}
				}
			} // done messages 

			// callback dialog
			if (d.peer_type == TG_PEER_TYPE_NULL) {
				ON_LOG(tg, "%s: can't find dialog data "
						"for peer: %.8x: %ld",
						__func__, peer->_id, peer->chat_id_);
				continue;
			}
			if (callback)
				callback(data, &d);

			// free dialog
			free(d.name);
		} // done dialogs
		
		// free tl
		/* TODO:  <29-11-24, yourname> */
		
		return md.dialogs_len;

	} else { // not dialogs or dialogsSlice
		// throw error
		char *err = tg_strerr(tl); 
		ON_ERR(tg, tl, "%s", err);
		free(err);
		// free tl
		/* TODO:  <29-11-24, yourname> */
	}
	return 0;
}

struct _sync_dialogs_update_dialog_t{
	tg_t *tg;
	int d;
	void *userdata;
  void (*on_done)(void *userdata);
};

static int _sync_dialogs_update_dialog(
		void *data, 
		const tg_dialog_t *dialog)
{
	if (!dialog)
		return 0;

	struct _sync_dialogs_update_dialog_t *d = data;
	d->d = dialog->top_message_date; 

	ON_LOG(d->tg, "%s: %s", __func__, dialog->name);

	// save dialog to database
	struct str s;
	str_init(&s);

	str_appendf(&s,
		"INSERT INTO \'dialogs\' (\'peer_id\') "
		"SELECT  "_LD_" "
		"WHERE NOT EXISTS (SELECT 1 FROM dialogs WHERE peer_id = "_LD_");\n"
		, dialog->peer_id, dialog->peer_id);

	str_appendf(&s, "UPDATE \'dialogs\' SET ");
	
	#define TG_DIALOG_STR(t, n, type, name) \
	if (dialog->n){\
		str_appendf(&s, "\'" name "\'" " = \'"); \
		str_append(&s, (char*)dialog->n, strlen((char*)dialog->n)); \
		str_appendf(&s, "\', "); \
	}
		
	#define TG_DIALOG_ARG(t, n, type, name) \
		str_appendf(&s, "\'" name "\'" " = "_LD_", ", (uint64_t)dialog->n);
	
	TG_DIALOG_ARGS
	#undef TG_DIALOG_ARG
	#undef TG_DIALOG_STR
	
	str_appendf(&s, "id = %d WHERE peer_id = "_LD_";\n"
			, d->tg->id, dialog->peer_id);
	
	/*ON_LOG(d->tg, "%s: %s", __func__, s.str);*/
	if (tg_sqlite3_exec(d->tg, s.str) == 0){
		// update hash
		update_hash(&d->tg->dialogs_hash, 
				        dialog->top_message_id);
	}
	free(s.str);

	return 0;
}

void tg_sync_dialogs_to_database(tg_t *tg, int limit, int date,
		void *userdata, void (*on_done)(void *userdata))
{
	// create table
	char sql[BUFSIZ]; 
	
	#define TG_DIALOG_ARG(t, n, type, name) \
		sprintf(sql, "ALTER TABLE \'dialogs\' ADD COLUMN "\
				"\'" name "\' " type ";\n");\
		ON_LOG(tg, "%s", sql);\
		tg_sqlite3_exec(tg, sql);	
	#define TG_DIALOG_STR(t, n, type, name) \
		sprintf(sql, "ALTER TABLE \'dialogs\' ADD COLUMN "\
				"\'" name "\' " type ";\n");\
		ON_LOG(tg, "%s", sql);\
		tg_sqlite3_exec(tg, sql);	
	TG_DIALOG_ARGS
	#undef TG_DIALOG_ARG
	#undef TG_DIALOG_STR

  struct _sync_dialogs_update_dialog_t d = {
	.d = date,
	.tg = tg,
	.on_done = on_done,
	.userdata = userdata,
  };

  int ret = limit;
  while (ret >= limit){
		ret = tg_get_dialogs(tg, limit>0 ? limit : 10,
				d.d, &tg->dialogs_hash,
			 	NULL, 
				&d,
			 	_sync_dialogs_update_dialog);
		
	  // update hash
	  dialogs_hash_to_database(
	    d.tg, d.tg->dialogs_hash);

		if (limit > 0)
			break;

	  // sleep
		sleep(2);
	}
  if (d.on_done)
	  d.on_done(userdata);
}

static void * _async_dialogs_thread(void * data)
{
	struct _sync_dialogs_update_dialog_t *d = data;
	ON_LOG(d->tg, "%s: start", __func__);

	ON_LOG(d->tg, "%s: updating dialogs...", __func__);	
	tg_sync_dialogs_to_database(
			d->tg, 0, time(NULL),
			d->userdata, d->on_done);

	d->tg->sync_dialogs = false;

	free(d);

	pthread_exit(0);	
}

void tg_async_dialogs_to_database(tg_t *tg,
		void *userdata, void (*on_done)(void *userdata))
{
	if (tg->sync_dialogs)
		return;

	tg->sync_dialogs = true;

	// open socket
	tg->sync_dialogs_sockfd = 
		tg_net_open_port(tg, 80);

	// set data
	struct _sync_dialogs_update_dialog_t *d = 
		NEW(struct _sync_dialogs_update_dialog_t, return);
	d->tg = tg;
	d->d = time(NULL);
	d->userdata = userdata;
	d->on_done = on_done;
	
	//create new thread
	if (pthread_create(
			&(tg->sync_dialogs_tid), 
			NULL, 
			_async_dialogs_thread, 
			d))
		ON_ERR(tg, NULL, "%s: can't create thread", __func__);
}

int tg_get_dialogs_from_database(
		tg_t *tg,
		void *data,
		int (*callback)(void *data, const tg_dialog_t *dialog))
{
	struct str s;
	str_init(&s);
	str_appendf(&s, "SELECT ");
	
	#define TG_DIALOG_ARG(t, n, type, name) \
		str_appendf(&s, name ", ");
	#define TG_DIALOG_STR(t, n, type, name) \
		str_appendf(&s, name ", ");
	TG_DIALOG_ARGS
	#undef TG_DIALOG_ARG
	#undef TG_DIALOG_STR
	
	str_appendf(&s, 
			"id FROM dialogs WHERE id = %d " 
			//"ORDER BY \'pinned\' DESC, \'top_message_date\' DESC;", tg->id);
			"ORDER BY \'top_message_date\' DESC;", tg->id);
		
	tg_sqlite3_for_each(tg, s.str, stmt){
		tg_dialog_t d;
		memset(&d, 0, sizeof(d));
		
		int col = 0;
		#define TG_DIALOG_ARG(t, n, type, name) \
			d.n = sqlite3_column_int64(stmt, col++);
		#define TG_DIALOG_STR(t, n, type, name) \
			d.n = strndup(\
				(char *)sqlite3_column_text(stmt, col),\
				sqlite3_column_bytes(stmt, col));\
			col++;
		
		TG_DIALOG_ARGS
		#undef TG_DIALOG_ARG
		#undef TG_DIALOG_STR

		if (callback){
			if (callback(data, &d)){
				sqlite3_close(db);
				break;
			}
		}
	}	
	
	free(s.str);
	return 0;
}
