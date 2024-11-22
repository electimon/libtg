/**
 * File              : cry.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 21.11.2024
 * Last Modified Date: 22.11.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#include "rsa.h"
#include "aes.h"
#include "buf.h"

buf_t tl_cry_rsa_e(buf_t b)
{
  buf_t r = {};
	buf_init(&r);
	if (b.size > r.asize)
		buf_realloc(&r, b.size);

	r.size = 
		tl_rsa(b.data, b.size, r.data);

  return r;
}

buf_t tl_cry_aes_e(buf_t b, buf_t key, buf_t iv)
{
	//printf("%s\n", __func__);
  buf_t r = {};
	buf_init(&r);
	if (b.size > r.size)
		buf_realloc(&r, b.size * 3);

	int l = aes_e(b.data, r.data, b.size, key.data, iv.data);

  if (!l) {
    printf("error during aes encryption");
  }

  r.size = l;

	//printf("%s done\n", __func__);
  return r;
}

buf_t tl_cry_aes_d(buf_t b, buf_t key, buf_t iv)
{
	//printf("%s\n", __func__);
  buf_t r = {};
	buf_init(&r);
	if (b.size > r.size)
		buf_realloc(&r, b.size * 2);
  
	int l = aes_d(b.data, r.data, b.size, key.data, iv.data);

  if (!l) {
    printf("error during aes decryption");
  }

  r.size = l;

	//printf("%s done\n", __func__);
  return r;
}
