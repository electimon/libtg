/**
 * File              : net.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 21.11.2024
 * Last Modified Date: 23.11.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#ifndef TL_NET_H
#define TL_NET_H

#include <stdint.h>
#include "../libtg.h"

extern int tg_net_open(tg_t*);
extern void tg_net_close(tg_t*);
extern void tg_net_send(tg_t*, const buf_t);
extern buf_t tg_net_receive(tg_t*);

#endif /* defined(TL_NET_H) */
