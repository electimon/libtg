#ifndef TG_PEER_H
#define TG_PEER_H

#include "tg.h"

typedef enum {
	TG_PEER_TYPE_NULL = 0,
	TG_PEER_TYPE_USER = id_peerUser,
	TG_PEER_TYPE_CHANNEL = id_peerChannel,
	TG_PEER_TYPE_CHAT = id_peerChat,
} TG_PEER_TYPE;

typedef struct tg_peer_t_ {
	TG_PEER_TYPE type;
	uint64_t id;
	uint64_t access_hash;
} tg_peer_t;

buf_t tg_inputPeer(tg_peer_t peer);
buf_t tg_peer(tg_peer_t peer);

tg_peer_t tg_peer_by_phone(tg_t *tg, const char *phone);

#endif /* ifndef TG_PEER_H */
