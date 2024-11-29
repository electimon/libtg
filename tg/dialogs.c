/**
 * File              : dialogs.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 29.11.2024
 * Last Modified Date: 29.11.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#include "tg.h"
#include "../tl/id.h"
#include "../mtx/include/net.h"
#include <stdlib.h>
#include <string.h>
#include "time.h"

int tg_get_dialogs(tg_t *tg, int top_msg_id, int limit, 
		time_t date, long * hash, int *folder_id, void *data,
		int (*callback)(void *data, 
			const tg_dialog_t *dialog))
{
	int i, id=top_msg_id;
	long h = 0;
	if (hash)
		h = *hash;

	InputPeer inputPeer = tl_inputPeerSelf();
	time_t t = time(NULL);
	if (date)
		t = date;

	buf_t getDialogs = 
		tl_messages_getDialogs(
				NULL,
				folder_id, 
				0,
				id, 
				&inputPeer, 
				limit,
				h);

	tl_t *tl = tg_send_query(tg, getDialogs);
	if (tl && tl->_id == id_messages_dialogsNotModified){
		ON_LOG(tg, "%s: dialogs not modified", __func__);
		return 0;
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
		
		int k;
		for (k = 0;  k< md.messages_len; ++k) {
			tl_message_t *m = (tl_message_t *)md.messages_[k];
			h = h ^ (h >> 21);
			h = h ^ (h << 35);
			h = h ^ (h >> 4);
			h = h + m->id_;
			id = m->id_;
		}
		if (hash)
			*hash = h;

		printf("DIALOGS: %d\n", md.dialogs_len);

		for (i = 0; i < md.dialogs_len; ++i) {
			// handle dialogs
			tg_dialog_t d;
			memset(&d, 0, sizeof(d));
			if (md.dialogs_[i]->_id != id_dialog){
				ON_LOG(tg, "%s: unknown dialog type: %.8x",
						__func__, md.dialogs_[i]->_id);
				continue;
			}
			tl_dialog_t *dialog = (tl_dialog_t *)md.dialogs_[i];	
			
			d.dialog = (tl_dialog_t *)md.dialogs_[i];
			
			tl_peerChat_t *peer = (tl_peerChat_t *)dialog->peer_;
			d.id = peer->chat_id_;
			
			int k;

			// iterate users
			for (k = 0; k < md.users_len; ++k) {
				if (md.users_[k]->_id == id_user){
					tl_user_t *user = 
						(tl_user_t *)md.users_[k];
					if (d.id == user->id_){
						d.type = TG_DIALOG_TYPE_USER;
						d.tl = (tl_t *)user;
						d.name = 
							strndup((char *)user->username_.data,
									user->username_.size);

					}
				} else if (md.users_[k]->_id == id_userEmpty){
					tl_userEmpty_t *user =
						(tl_userEmpty_t *)md.users_[k];
					if (d.id == user->id_){
						d.type = TG_DIALOG_TYPE_USER_EMPTY;
						d.tl = (tl_t *)user;
						d.name = strdup("empty"); 
					}
				}	
			} // done users

			// iterate chats
			for (k = 0; k < md.chats_len; ++k) {
				if (md.chats_[k]->_id == id_channel){
					tl_channel_t *channel = 
						(tl_channel_t *)md.chats_[k];
					if (d.id == channel->id_){
						d.type = TG_DIALOG_TYPE_CHANNEL;
						d.tl = (tl_t *)channel;
						d.name = 
							strndup((char *)channel->title_.data,
									channel->title_.size);
					}
				}
				else if (md.chats_[k]->_id == id_channelForbidden){
					tl_channelForbidden_t *channel = 
						(tl_channelForbidden_t *)md.chats_[k];
					if (d.id == channel->id_){
						d.type = TG_DIALOG_TYPE_CHANNEL_FORBIDEN;
						d.tl = (tl_t *)channel;
						d.name = 
							strndup((char *)channel->title_.data,
									channel->title_.size);
					}
				}
				else if (md.chats_[k]->_id == id_chat){
					tl_chat_t *chat = 
						(tl_chat_t *)md.chats_[k];
					if (d.id == chat->id_){
						d.type = TG_DIALOG_TYPE_CHAT;
						d.tl = (tl_t *)chat;
						d.name = 
							strndup((char *)chat->title_.data,
									chat->title_.size);
					}
				}
				else if (md.chats_[k]->_id == id_chatForbidden){
					tl_chatForbidden_t *chat = 
						(tl_chatForbidden_t *)md.chats_[k];
					if (d.id == chat->id_){
						d.type = TG_DIALOG_TYPE_CHAT_FORBIDEN;
						d.tl = (tl_t *)chat;
						d.name = 
							strndup((char *)chat->title_.data,
									chat->title_.size);
					}
				}
			} // done chats
		
			// iterate messages
			for (k = 0; k < md.messages_len; ++k){
				if (md.messages_[k]->_id == id_message){
					tl_message_t *message = 
						(tl_message_t *)md.messages_[k];
					if (message->id_ == dialog->top_message_){
						d.top_message = message;
					}
				}
			} // done messages 
			
			// callback dialog
			if (d.type == TG_DIALOG_TYPE_NULL) {
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
		
		return 0;

	} else { // not dialogs or dialogsSlice
		// throw error
		char *err = tg_strerr(tl); 
		ON_ERR(tg, tl, "%s", err);
		free(err);
		// free tl
		/* TODO:  <29-11-24, yourname> */
	}
	return 1;
}
