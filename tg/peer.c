#include "peer.h"

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
