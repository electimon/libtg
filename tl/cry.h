/**
 * File              : cry.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 21.11.2024
 * Last Modified Date: 21.11.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#ifndef TL_CRY_H
#define TL_CRY_H

#include "buf.h"

extern buf_t tl_cry_rsa_e(buf_t b);
extern buf_t tl_cry_aes_e(buf_t b, buf_t key, buf_t iv);
extern buf_t tl_cry_aes_d(buf_t b, buf_t key, buf_t iv);

#endif /* defined(TL_CRY_H) */
