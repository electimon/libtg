#include "../tg/tg.h"
#include "cry.h"
#include "hsh.h"
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bn.h>
#include <assert.h>
#include "../tl/serialize.h"

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

uint64_t tg_cry_rsa_fpt(tg_t *tg){
 /* key fingerprint (64 lower-order bits of SHA1
	* (server_public_key); the public key is represented as a
	* bare type rsa_public_key n:string e:string =
	* RSAPublicKey, where, as usual, n and e are numbers in
	* big endian format serialized as strings of bytes,
	* following
	* which SHA1 is computed) received by the server. */

	RSA *rsa = read_pubkey(tg);
	if (!rsa)
		return 1;

	const BIGNUM *n = RSA_get0_n(rsa);
	const BIGNUM *e = RSA_get0_e(rsa);

	buf_t a;
	buf_init(&a);
	a.size = BN_bn2bin(n, a.data);
	buf_t astr = serialize_bytes(a.data, a.size);
	buf_free(a);

	buf_t b;
	buf_init(&b);
	b.size = BN_bn2bin(e, b.data);
	buf_t bstr = serialize_bytes(b.data, b.size);
	buf_free(b);

	buf_t buf = buf_cat(astr, bstr);
	buf_t buf_hash = tg_hsh_sha1(buf);
	buf_free(astr); buf_free(bstr);

	// get lower 64bit
	int c = 20 - 8; // 160 - 64 SHA1 has 164 bit
	buf_t lower = 
		buf_add(buf_hash.data + c, 8);
	buf_free(buf_hash);

	uint64_t fingerprint = buf_get_ui64(lower);
	buf_free(lower);

	return fingerprint;
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
  assert(BN_mod_exp(r, a, e, n, BN_ctx)); // r = a^e % n
	int len = BN_bn2bin(a, (unsigned char *) ret.data);
	ret.size = len;

	BN_free(a);
	BN_free(r);
	RSA_free(rsa);

	return ret;
}
