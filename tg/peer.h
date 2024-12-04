#ifndef TG_PEER_H
#define TG_PEER_H

#include "tg.h"

typedef enum {
	TG_PEER_TYPE_NULL = 0,
	TG_PEER_TYPE_USER = id_peerUser,
	TG_PEER_TYPE_CHANNEL = id_peerChannel,
	TG_PEER_TYPE_CHAT = id_peerChat,
} TG_PEER_TYPE;

buf_t tg_inputPeer(TG_PEER_TYPE type, uint64_t id, uint64_t access_hash);
buf_t tg_peer(TG_PEER_TYPE type, uint64_t id);

#endif /* ifndef TG_PEER_H */
