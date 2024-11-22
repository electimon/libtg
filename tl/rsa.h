/**
 * File              : rsa.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 21.11.2024
 * Last Modified Date: 22.11.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#ifndef TL_RSA_H
#define TL_RSA_H

#include <stddef.h>
int tl_rsa_cmp_to_pubkey_modulus(unsigned char *d, size_t len);

extern int 
tl_rsa(unsigned char * from, size_t from_size, unsigned char * to);
//extern void tl_rsa(unsigned char * from, size_t from_size, unsigned char * to, size_t to_size);
extern void tl_rand_bytes(unsigned char * s, int l);
extern int tl_pow_mod(unsigned char * y, unsigned char * g, size_t g_s, unsigned char * e, size_t e_s, unsigned char * m, size_t m_s);

#endif /* defined(TL_RSA_H) */
