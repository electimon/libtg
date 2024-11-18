//
//  enl.c
//  mtx
//
//  Created by Pavel Morozkin on 18.01.14.
//  Copyright (c) 2014 Pavel Morozkin. All rights reserved.
//

#include "../include/api.h"
#include "../include/enl.h"
#include "../include/buf.h"
#include "../include/sha256.h"
#include "../include/hsh.h"
#include <stdlib.h>

buf_t enl_encrypt(buf_t b, msg_t t)
{
	/*printf("%s\n", __func__);*/
  buf_t e = {};
	buf_init(&e);

  switch (t) {
    case API:
    {
      buf_t auth_key = shared_rc_get_key();
			// For MTProto 2.0, SHA1 is still used here, because
			// auth_key_id should identify the authorization key
			// used independently of the protocol
			// version.
      buf_t key_hash = api.hsh.sha1(auth_key);
      buf_t auth_key_id = api.buf.add(key_hash.data + 12, 8);
      
			// ui32_t pad_ = (16 - (b.size % 16)) % 16;
			// 12..1024 padding bytes are used instead of 0..15
			// padding bytes in v.1.0
			ui32_t pad_ =  16 + (16 - (b.size % 16)) % 16;
			//Padding bytes are involved in the computation of
/* 
For MTProto 2.0, the algorithm for computing aes_key and
aes_iv from auth_key and msg_key is as follows.

  • msg_key_large = SHA256 (substr (auth_key, 88+x, 32) + plaintext + random_padding);
  • msg_key = substr (msg_key_large, 8, 16);
  • sha256_a = SHA256 (msg_key + substr (auth_key, x, 36));
  • sha256_b = SHA256 (substr (auth_key, 40+x, 36) + msg_key);
  • aes_key = substr (sha256_a, 0, 8) + substr (sha256_b, 8, 16) + substr (sha256_a, 24, 8);
  • aes_iv = substr (sha256_b, 0, 8) + substr (sha256_a, 8, 16) + substr (sha256_b, 24, 8);

where x = 0 for messages from client to server and x = 8 for those from server to client.
*/

			//msg_key_large = SHA256 (substr (auth_key, 88+x, 32) 
			//+ plaintext + random_padding);
			buf_t msg_key_large_ = 
				buf_add(auth_key.data + 88, 32);
			msg_key_large_ = buf_cat(msg_key_large_, b);
      buf_t pad = buf_rand(pad_);
			msg_key_large_ = buf_cat(msg_key_large_, pad);
			buf_t msg_key_large = hsh_sha256(msg_key_large_);

			// msg_key = substr (msg_key_large, 8, 16);
			buf_t msg_key = 
				buf_add(msg_key_large.data + 8, 16);

			//sha256_a = SHA256 (msg_key + substr (auth_key, x, 36));
			buf_t sha256_a_ = 
				buf_add(msg_key.data, msg_key.size);
			sha256_a_ = 
				buf_cat_data(sha256_a_, auth_key.data, 36);
			buf_t sha256_a = hsh_sha256(sha256_a_);


			//sha256_b = SHA256 (substr (auth_key, 40+x, 36) + msg_key);
			buf_t sha256_b_ = 
				buf_add(auth_key.data + 40, 36);
			sha256_b_ = 
				buf_cat_data(sha256_b_, msg_key.data, msg_key.size);
			buf_t sha256_b = hsh_sha256(sha256_b_);

			//aes_key = substr (sha256_a, 0, 8) 
			//+ substr (sha256_b, 8, 16) + substr (sha256_a, 24, 8);
			buf_t aes_key = buf_add(sha256_a.data, 8);
			aes_key = buf_cat_data(aes_key, sha256_b.data + 8, 16);
			aes_key = buf_cat_data(aes_key, sha256_a.data + 24, 8);
			
			//aes_iv = substr (sha256_b, 0, 8) + 
			//substr (sha256_a, 8, 16) + substr (sha256_b, 24, 8);
			buf_t aes_iv = buf_add(sha256_b.data, 8);
			aes_iv = buf_cat_data(aes_iv, sha256_a.data + 8, 16);
			aes_iv = buf_cat_data(aes_iv, sha256_b.data + 24, 8);
			
			//buf_t data_hash = api.hsh.sha1(b);
      //buf_t msg_key = api.buf.add(data_hash.data + 4, 16);
      //e = api.buf.cat(e, msg_key);
      //ui32_t pad_ = (16 - (b.size % 16)) % 16;

      //if (pad_) {
        //buf_t pad = {};
				//buf_init(&pad);
        //pad.size = pad_;
        //b = api.buf.cat(b, pad);
      //}

      //aes_params_t p = 
				//enl_get_params(key, msg_key, MODE_C2S);
      //buf_t enc = api.cry.aes_e(b, p.aes_key, p.aes_iv);

			// Encrypted Message: encrypted_data
			// Contains the cypher text for the following data:
			//
			// salt  session_id message_id seq_no message_data_length  message_data padding12..1024
			// int64 int64      int64      int32  int32                bytes        bytes
			buf_t data = buf_add(b.data, b.size); 
			data = buf_cat(data, pad);
      buf_t enc = api.cry.aes_e(data, aes_key, aes_iv);
      
			// Encrypted Message
			// auth_key_id msg_key encrypted_data
			// int64       int128  bytes
      e = api.buf.cat(e, auth_key_id);
      e = api.buf.cat(e, msg_key);
			e = api.buf.cat(e, enc);

			// free
			free(key_hash.data);
			free(auth_key_id.data);
			free(msg_key_large_.data);
			free(pad.data);
			free(msg_key_large.data);
			free(msg_key.data);
			free(sha256_a_.data);
			free(sha256_a.data);
			free(sha256_b_.data);
			free(sha256_b.data);
			free(aes_key.data);
			free(aes_iv.data);
			free(data.data);
			free(enc.data);

      break;
    }
    case RFC:
    {
      e = api.buf.cat(e, b);
      break;
    }
    default:
    {
      api.log.error("unknown type");

      break;
    }
  }

  return e;
}

buf_t enl_decrypt(buf_t m, msg_t t)
{
	/*printf("%s\n", __func__);*/
  buf_t d;
	buf_init(&d);

	if (!m.size) {
    api.log.error("enl_decrypt: received nothing");
		return d;
  }

  switch (t) {
    case API:
    {
      buf_t auth_key = shared_rc_get_key();
      buf_t key_hash = api.hsh.sha1(auth_key);
      buf_t auth_key_id  = api.buf.add(key_hash.data + 12, 8);
      buf_t auth_key_id_ = api.buf.add(m.data, 8);
      m = api.buf.add(m.data + 8, m.size - 8);

      if (!api.buf.cmp(auth_key_id, auth_key_id_)) {
        api.log.error("different auth key id's");
      }

      buf_t msg_key = api.buf.add(m.data, 16);
      m = api.buf.add(m.data + 16, m.size - 16);
      //aes_params_t p = enl_get_params(key, msg_key, MODE_S2C);
			
			//sha256_a = SHA256 (msg_key + substr (auth_key, x, 36));
			buf_t sha256_a_ = 
				buf_add(msg_key.data, msg_key.size);
			sha256_a_ = 
				buf_cat_data(sha256_a_, auth_key.data + 8, 36);
			buf_t sha256_a = hsh_sha256(sha256_a_);

			//sha256_b = SHA256 (substr (auth_key, 40+x, 36) + msg_key);
			buf_t sha256_b_ = 
				buf_add(auth_key.data + 40+8, 36);
			sha256_b_ = 
				buf_cat_data(sha256_b_, msg_key.data, msg_key.size);
			buf_t sha256_b = hsh_sha256(sha256_b_);
			
			//aes_key = substr (sha256_a, 0, 8) 
			//+ substr (sha256_b, 8, 16) + substr (sha256_a, 24, 8);
			buf_t aes_key = buf_add(sha256_a.data, 8);
			aes_key = buf_cat_data(aes_key, sha256_b.data + 8, 16);
			aes_key = buf_cat_data(aes_key, sha256_a.data + 24, 8);
			
			//aes_iv = substr (sha256_b, 0, 8) + 
			//substr (sha256_a, 8, 16) + substr (sha256_b, 24, 8);
			buf_t aes_iv = buf_add(sha256_b.data, 8);
			aes_iv = buf_cat_data(aes_iv, sha256_a.data + 8, 16);
			aes_iv = buf_cat_data(aes_iv, sha256_b.data + 24, 8);
			
			d = api.cry.aes_d(m, aes_key, aes_iv);

      /*buf_t l_ = api.buf.add(d.data + 28, 4);*/
      /*ui32_t l = api.buf.get_ui32(l_);*/
      /*ui32_t pad_ = (16 - (l % 16)) % 16;*/

      /*if (pad_) {*/
        /*d = api.buf.add(d.data, d.size - pad_);*/
      /*}*/

      //buf_t data_hash = api.hsh.sha1(d);
      /*buf_t data_hash = hsh_sha256(d);*/
      /*buf_t msg_key_ = api.buf.add(data_hash.data + 4, 16);*/

      /*if (!api.buf.cmp(msg_key_, msg_key)) {*/
        /*api.log.error("different msg key's");*/
      /*}*/

      break;
    }
    case RFC:
    {
      buf_t key = {};
			buf_init(&key);
      key.size = 8;

      buf_t d_key = api.buf.add(m.data, 8);

      if (!api.buf.cmp(key, d_key)) {
        api.log.error("trl_transport: keys mismatch");
      }

      d = api.buf.add(m.data + 8, m.size - 8);

      break;
    }
    default:
    {
      break;
    }
  }

	/*printf("%s done\n", __func__);*/
  return d;
}


aes_params_t enl_get_params(buf_t auth_key, buf_t msg_key, get_params_mode_t m)
{
	printf("%s\n", __func__);
  ui8_t x = 0;

  switch (m) {
    case MODE_C2S:
    {
      x = 0;

      break;
    }
    case MODE_S2C:
    {
      x = 8;

      break;
    }
    default:
    {
      api.log.error("unknown mode");

      break;
    }
  }

  buf_t aes_key = {};
	buf_init(&aes_key);
  buf_t aes_iv = {};
	buf_init(&aes_iv);
  buf_t tmp = api.buf.add(msg_key.data, 16);
  tmp = api.buf.cat(tmp, api.buf.add(auth_key.data + x, 32));
  buf_t hash = api.hsh.sha1(tmp);
  aes_key = api.buf.cat(aes_key, api.buf.add(hash.data, 8));
  aes_iv = api.buf.cat(aes_iv, api.buf.add(hash.data + 8, 12));
  tmp = api.buf.add(auth_key.data + 32 + x, 16);
  tmp = api.buf.cat(tmp, api.buf.add(msg_key.data, 16));
  tmp = api.buf.cat(tmp, api.buf.add(auth_key.data + 48 + x, 16));
  hash = api.hsh.sha1(tmp);
  aes_key = api.buf.cat(aes_key, api.buf.add(hash.data + 8, 12));
  aes_iv = api.buf.cat(aes_iv, api.buf.add(hash.data, 8));
  tmp = api.buf.add(auth_key.data + 64 + x, 32);
  tmp = api.buf.cat(tmp, api.buf.add(msg_key.data, 16));
  hash = api.hsh.sha1(tmp);
  aes_key = api.buf.cat(aes_key, api.buf.add(hash.data + 4, 12));
  aes_iv = api.buf.cat(aes_iv, api.buf.add(hash.data + 16, 4));
  tmp = api.buf.add(msg_key.data, 16);
  tmp = api.buf.cat(tmp, api.buf.add(auth_key.data + 96 + x, 32));
  hash = api.hsh.sha1(tmp);
  aes_iv = api.buf.cat(aes_iv, api.buf.add(hash.data, 8));
  aes_params_t p;
  p.aes_key = aes_key;
  p.aes_iv = aes_iv;

  return p;
}
