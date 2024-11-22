/**
 * File              : net.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 21.11.2024
 * Last Modified Date: 21.11.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#ifndef TL_NET_H
#define TL_NET_H

#include "types.h"

extern int tl_net_open(const char * ip, uint32_t port);
extern void tl_net_close(int);
extern void tl_net_send(const buf_t);
extern buf_t tl_net_receive();

#endif /* defined(TL_NET_H) */
