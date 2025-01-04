#include "peer.h"
#include "tg.h"
#include <stdint.h>

buf_t tg_inputPeer(tg_peer_t peer)
{
	buf_t p;
	switch (peer.type) {
		case TG_PEER_TYPE_CHANNEL:
			p = tl_inputPeerChannel(
					peer.id, peer.access_hash);
			break;
		case TG_PEER_TYPE_CHAT:
			p = tl_inputPeerChat(peer.id);
			break;
		case TG_PEER_TYPE_USER:
			p = tl_inputPeerUser(
					peer.id, peer.access_hash);
			break;
		default:
			buf_init(&p);
			break;
	}
	return p;
}

buf_t tg_peer(tg_peer_t peer)
{
	buf_t p;
	switch (peer.type) {
		case TG_PEER_TYPE_CHANNEL:
			p = tl_peerChannel(peer.id);
			break;
		case TG_PEER_TYPE_CHAT:
			p = tl_peerChat(peer.id);
			break;
		case TG_PEER_TYPE_USER:
			p = tl_peerUser(peer.id);
			break;
		default:
			buf_init(&p);
			break;
	}
	return p;
}

tg_peer_t tg_peer_by_phone(tg_t *tg, const char *phone){
	tg_peer_t peer = {0, 0, 0};

	buf_t query = tl_contacts_resolvePhone(phone);
	tl_t *tl = tg_run_api(tg, &query);
	buf_free(query);
	if (tl != NULL && tl->_id == id_contacts_resolvedPeer)
	{
		tl_contacts_resolvedPeer_t *rp = 
			(tl_contacts_resolvedPeer_t *)tl;
		tl_peerUser_t *p = (tl_peerUser_t *)rp->peer_;
		if (p){
			if (p->_id == id_peerUser)
				peer.type = TG_PEER_TYPE_USER;
			else if (p->_id == id_peerChannel)
				peer.type = TG_PEER_TYPE_CHANNEL;
			else if (p->_id == id_peerChat)
				peer.type = TG_PEER_TYPE_CHAT;

			peer.id = p->user_id_;

			if (rp->users_len > 0){
				tl_user_t *user = (tl_user_t *)rp->users_[0];
				if (user){
					peer.access_hash = user->access_hash_;
				}
			}
		}
	}

	return peer;
}

static void tg_peer_color_set_to_colors(
		tg_t *tg,
		tg_colors_t *colors,
		tl_help_peerColorSet_t *pcs)
{
	if (pcs->_id != id_help_peerColorSet)
	{
		ON_ERR(tg, "%s: tl is not help_getPeerColorSet: %s",
				__func__, TL_NAME_FROM_ID(pcs->_id));
		return;
	}

	if (pcs->colors_ == NULL){
		ON_ERR(tg, "%s: colors is NULL",
				__func__);
		return;
	}
	
	int i;
	for (i = 0; i < pcs->colors_len; ++i) {
		switch (i) {
			case 0:
				colors->rgb0 = pcs->colors_[i];
				break;
			case 1:
				colors->rgb1 = pcs->colors_[i];
				break;
			case 2:
				colors->rgb1 = pcs->colors_[i];
				break;
			
			default:
				break;
		}	
	}
}

int tg_get_peer_profile_colors(tg_t *tg, uint32_t hash, 
		void *userdata,
		int (*callback)(void *userdata, 
			uint32_t color_id, tg_colors_t *colors, tg_colors_t *dark_colors))
{
	ON_LOG(tg, "%s: start", __func__);
	buf_t query = tl_help_getPeerProfileColors(0);
	tl_t *tl = tg_run_api(tg, &query);
	buf_free(query);
	if (tl != NULL && tl->_id == id_help_peerColors)
	{	
		tl_help_peerColors_t *hpc =
			(tl_help_peerColors_t *)tl;

		if (hpc->colors_ == NULL)
			return 0;

		int i, c=0;
		for (i = 0; i < hpc->colors_len; ++i) {
			tl_help_peerColorOption_t *pco = 
				(tl_help_peerColorOption_t *)hpc->colors_[i];
			if (pco->_id != id_help_peerColorOption)
				continue;
			
			tg_colors_t colors;
			memset(&colors, 0, sizeof(colors));
			tg_colors_t dark_colors;
			memset(&dark_colors, 0, sizeof(dark_colors));

			if (pco->colors_){
				tg_peer_color_set_to_colors(
						tg, 
						&colors, 
						(tl_help_peerColorSet_t *)pco->colors_);
			}
			
			if (pco->dark_colors_){
				tg_peer_color_set_to_colors(
						tg, 
						&dark_colors, 
						(tl_help_peerColorSet_t *)pco->dark_colors_);
			}

			// run callback
			if (callback)
				if (callback(userdata, pco->color_id_, &colors, &dark_colors))
					return ++c;

			c++;
		}
		return c;
	}

	ON_ERR(tg, "%s: tl is not help_peerColors: %s",
			__func__, TL_NAME_FROM_ID(tl->_id));
	
	return 0;
}
