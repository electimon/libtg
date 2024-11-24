/**
 * File              : cry.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 21.11.2024
 * Last Modified Date: 23.11.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#ifndef TL_CRY_H
#define TL_CRY_H

#include "../libtg.h"

extern int   tg_cry_rsa_cmp(tg_t*, buf_t);
extern buf_t tg_cry_rsa_enc(tg_t*, buf_t);
extern buf_t tg_cry_aes_e(buf_t b, buf_t key, buf_t iv);
extern buf_t tg_cry_aes_d(buf_t b, buf_t key, buf_t iv);

#endif /* defined(TL_CRY_H) */
