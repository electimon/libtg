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
