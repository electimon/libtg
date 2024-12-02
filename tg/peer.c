#include "peer.h"

buf_t tg_inputPeer(TG_PEER_TYPE t, long id, long h)
{
	buf_t p;
	switch (t) {
		case TG_PEER_TYPE_NULL:
			buf_init(&p);
			break;
		case TG_PEER_TYPE_CHANNEL:
			p = tl_inputPeerChannel(id, h);
			break;
		case TG_PEER_TYPE_CHAT:
			p = tl_inputPeerChat(id);
			break;
		case TG_PEER_TYPE_USER:
			p = tl_inputPeerUser(id, h);
			break;
		default:
			break;
	}
	return p;
}

buf_t tg_peer(TG_PEER_TYPE t, long id)
{
	buf_t p;
	switch (t) {
		case TG_PEER_TYPE_NULL:
			buf_init(&p);
			break;
		case TG_PEER_TYPE_CHANNEL:
			p = tl_peerChannel(id);
			break;
		case TG_PEER_TYPE_CHAT:
			p = tl_peerChat(id);
			break;
		case TG_PEER_TYPE_USER:
			p = tl_peerUser(id);
			break;
		default:
			break;
	}
	return p;
}
