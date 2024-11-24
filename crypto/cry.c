/**
 * File              : cry.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 21.11.2024
 * Last Modified Date: 23.11.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#include "../tg/tg.h"
#include "cry.h"
#include <openssl/aes.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bn.h>
//#include <openssl/rand.h>
#include <assert.h>

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

static RSA *read_pubkey(tg_t *tg)
{
	FILE * pub = fopen(tg->pubkey, "r");
  if (pub == NULL) {
    ON_ERR(tg, NULL,
				"%s: can not read public key from file: '%s'",
				 __func__, tg->pubkey);
		return NULL;
  }

  RSA * rsa = 
		PEM_read_RSAPublicKey(pub, NULL, NULL, NULL);
  if (!rsa) {
		fclose(pub);
    RSA_free(rsa);
    ON_ERR(tg, NULL,
				"%s: PEM_read_RSAPublicKey: '%s' returns NULL",
				__func__, tg->pubkey);
		return NULL;
  }

	fclose(pub);
	return rsa;
}

int tg_cry_rsa_cmp(tg_t *tg, buf_t buf)
{
	RSA *rsa = read_pubkey(tg);
	if (!rsa)
		return 1;

	BIGNUM *a = BN_new();
  BN_bin2bn(buf.data, buf.size, a);

	const BIGNUM *b = RSA_get0_n(rsa);
	RSA_free(rsa);
	
	int cmp = BN_cmp(a, b);
	
	BN_free(a);
	return cmp;
}

buf_t tg_cry_rsa_enc(tg_t *tg, buf_t buf)
{
	buf_t ret;
	buf_init(&ret);
	if (buf.size > ret.asize)
		buf_realloc(&ret, buf.size + 1);
	
	RSA *rsa = read_pubkey(tg);
	if (!rsa)
		return ret;
	
	BIGNUM *r = BN_new();

	BIGNUM *a = BN_new();
  BN_bin2bn(buf.data, buf.size, a);
	
	const BIGNUM *n = RSA_get0_n(rsa);
	const BIGNUM *e = RSA_get0_e(rsa);
  
	BN_CTX * BN_ctx = BN_CTX_new();
  assert(BN_mod_exp(r, a, e, n, BN_ctx)); // y = x^E % N
  int len = BN_bn2bin(a, (unsigned char *) ret.data);
	ret.size = len;

	BN_free(a);
	BN_free(r);
	RSA_free(rsa);

	return ret;
}

buf_t tg_cry_aes_e(buf_t b, buf_t k, buf_t iv)
{
  buf_t r;
	buf_init(&r);
	if (b.size > r.size)
		buf_realloc(&r, b.size * 3);
  
	AES_KEY key;
  AES_set_encrypt_key(
			k.data, 256, &key);

  AES_ige_encrypt(
			b.data, 
			r.data, 
			b.size, 
			&key, 
			iv.data, 
			AES_DECRYPT);

  r.size = b.size;
  return r;
}

buf_t tg_cry_aes_d(buf_t b, buf_t k, buf_t iv)
{
	buf_t r;
	buf_init(&r);
	if (b.size > r.size)
		buf_realloc(&r, b.size * 3);
  
	AES_KEY key;
  AES_set_decrypt_key(
			k.data, 256, &key);

  AES_ige_encrypt(
			b.data, 
			r.data, 
			b.size, 
			&key, 
			iv.data, 
			AES_DECRYPT);

  r.size = b.size;
  return r;
}

//void tg_rand_bytes(unsigned char * s, int l)
//{
  //RAND_bytes(s, l);
//}
