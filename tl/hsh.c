/**
 * File              : hsh.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 21.11.2024
 * Last Modified Date: 21.11.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#include "sha1.h"
#include "sha256.h"
#include "hsh.h"

buf_t tl_hsh_sha1(buf_t b)
{
  buf_t h;
	buf_init(&h);
  sha1(b.data, b.size, h.data);
  h.size = 20;

  return h;
}

buf_t tl_hsh_sha256(buf_t b)
{
  buf_t h;
	buf_init(&h);
  sha256_bytes(b.data, b.size, h.data);
  h.size = 256;

  return h;
}
