/**
 * File              : aes.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 21.11.2024
 * Last Modified Date: 21.11.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#ifndef TL_AES_H
#define TL_AES_H

#include <openssl/aes.h>

extern int aes_d(const unsigned char * in, unsigned char * out, const unsigned int l, const unsigned char * k, unsigned char * iv);
extern int aes_e(const unsigned char * in, unsigned char * out, const unsigned int l, unsigned char * k, unsigned char * iv);

#endif /* defined(TL_AES_H) */
