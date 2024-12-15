/**
 * File              : net.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 21.11.2024
 * Last Modified Date: 15.12.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#ifndef TL_NET_H
#define TL_NET_H

#include <stdint.h>
#include "../libtg.h"

extern int   tg_net_open(tg_t*);
extern int   tg_net_open_port(tg_t*, int port);
extern void  tg_net_close(tg_t*,int);
extern int  tg_net_send(tg_t*, int, const buf_t);
extern buf_t tg_net_receive(tg_t*, int);

extern int tg_net_add_query(tg_t *tg, const buf_t buf, uint64_t msg_id, 
		void *on_donep, int (*on_done)(void *on_donep, const buf_t buf),
		void *chunkp, 
		buf_t (*chunk)(void *chunkp, uint32_t received, uint32_t total));

extern int tg_net_send_queue_node(tg_t *tg);
#endif /* defined(TL_NET_H) */
