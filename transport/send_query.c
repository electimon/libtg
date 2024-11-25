#include <stdint.h>
#define _POSIX_C_SOURCE 199309L

#include <time.h>
#include <assert.h>
#include <stdbool.h>

#include "../tg/tg.h"
#include "../tl/alloc.h"
#include "../crypto/hsh.h"
#include "../crypto/cry.h"
#include "../transport/net.h"

static void tg_my_clock_gettime(int clock_id, struct timespec * T)
{
  assert(clock_gettime(clock_id, T) >= 0);
}

static double tg_get_utime(int clock_id)
{
  struct timespec T;
  tg_my_clock_gettime(clock_id, &T);
  double res = T.tv_sec + (double) T.tv_nsec * 1e-9;
  return res;
}

static long long tg_get_current_time()
{
	return 
		(long long)((1LL << 32) * 
				tg_get_utime(CLOCK_REALTIME)) & -4;
}

static buf_t header(tg_t *tg, buf_t b, bool enc)
{
  buf_t s = {};
	buf_init(&s);

  if (enc) {
		// salt  session_id message_id seq_no message_data_length  message_data padding12..1024
		// int64 int64      int64      int32  int32                bytes        bytes
		
		// salt
		s = buf_cat(s, tg->salt);
		
		//session_id
		s = buf_cat(s, tg->ssid);
		
		//message_id
		s = buf_cat_ui64(s, tg_get_current_time());
		
		//seq_no
		s = buf_cat_ui32(s, tg->seqn);
		
		//message_data_length
		s = buf_cat_ui32(s, b.size);
		
		//message_data
		s = buf_cat(s, b);
		
		//padding
		uint32_t pad =  16 + (16 - (b.size % 16)) % 16;
		s = buf_cat_rand(s, pad);

	} else {
		//auth_key_id = 0 message_id message_data_length message_data
		//int64           int64      int32               bytes

		//auth_key_id
		s = buf_cat_ui64(s, 0);
		
		//message_id
		s = buf_cat_ui64(s, tg_get_current_time());
		
		//message_data_length
		s = buf_cat_ui32(s, b.size);
		
		// message_data
		s = buf_cat(s, b);
  }

	ON_LOG_BUF(tg, s, "%s: ", __func__);
  return s;
}

static buf_t deheader(tg_t *tg, buf_t b, bool enc)
{
	if (!b.size){
		ON_ERR(tg, NULL, "%s: got nothing", __func__);
		return b;
	}

  buf_t d;
	buf_init(&d);

  if (enc){
		// salt  session_id message_id seq_no message_data_length  message_data padding12..1024
		// int64 int64      int64      int32  int32                bytes        bytes
		
		// salt
		uint64_t salt = deserialize_ui64(&b);
		// update server salt
		tg->salt = buf_add_ui64(salt);
		
		// session_id
		uint64_t ssid = deserialize_ui64(&b);
		// check ssid
		if (ssid != buf_get_ui64(tg->ssid)){
			ON_ERR(tg, NULL, "%s: session id mismatch!", __func__);
		}

		// message_id
		uint64_t msg_id = deserialize_ui64(&b);
		tg_add_mgsid(tg, msg_id);
		
		// seq_no
		uint64_t seq_no = deserialize_ui32(&b);

		// data len
		uint64_t msg_data_len = deserialize_ui32(&b);
		// set data len without padding
		b.size = msg_data_len;
  
	} else {
	//auth_key_id = 0 message_id message_data_length message_data
	//int64           int64      int32               bytes
		
		// auth_key_id
		uint64_t auth_key_id = buf_get_ui64(b);
		if (auth_key_id != 0){
			ON_ERR(tg, NULL, 
					"%s: auth_key_id is not 0 for unencrypted message", __func__);
			return b;
		}
		auth_key_id = deserialize_ui64(&b);

		// message_id
		uint64_t msg_id = deserialize_ui64(&b);
		tg_add_mgsid(tg, msg_id);
		// ack msg_id (under construction...)

		// message_data_length
		uint32_t msg_data_len = deserialize_ui32(&b);

		d = buf_cat(d, b);

		// check len matching
		if (msg_data_len != b.size){
			ON_ERR(tg, NULL, 
					"%s: msg_data_len mismatch: expected: %d, got: %d", 
					__func__, msg_data_len, b.size);
		}
	}
    
	ON_LOG_BUF(tg, d, "%s: ", __func__);
  return d;
}

static buf_t encrypt(tg_t *tg, buf_t b, bool enc)
{
  buf_t e = {};
	buf_init(&e);

  if (enc) {
		// For MTProto 2.0, SHA1 is still used here, because
		// auth_key_id should identify the authorization key
		// used independently of the protocol version.
		buf_t key_hash = tg_hsh_sha1(tg->key);
		buf_t auth_key_id = 
			buf_add(key_hash.data + 12, 8);
		buf_free(key_hash);
      
	/* For MTProto 2.0, the algorithm for computing aes_key 
	 * and aes_iv from auth_key and msg_key is as follows.
	 * • msg_key_large = SHA256 (substr (auth_key, 88+x, 32) + 
	 *   plaintext + random_padding);
	 * • msg_key = substr (msg_key_large, 8, 16);
	 * • sha256_a = SHA256 (msg_key + substr (auth_key, x, 36));
	 * • sha256_b = SHA256 (substr (auth_key, 40+x, 36) + msg_key);
	 * • aes_key = substr (sha256_a, 0, 8) + 
	 *   substr (sha256_b, 8, 16) + substr (sha256_a, 24, 8);
	 * • aes_iv = substr (sha256_b, 0, 8) + 
	 * substr (sha256_a, 8, 16) + substr (sha256_b, 24, 8);
	 * where x = 0 for messages from client to server 
	 * and x = 8 for those from server to client. */

		//msg_key_large = SHA256 (substr (auth_key, 88+x, 32) 
		//+ plaintext + random_padding);
		buf_t msg_key_large_ = 
			buf_add(tg->key.data + 88, 32);
		msg_key_large_ = buf_cat(msg_key_large_, b);
		buf_t msg_key_large = tg_hsh_sha256(msg_key_large_);
		buf_free(msg_key_large_);

		// msg_key = substr (msg_key_large, 8, 16);
		buf_t msg_key = 
			buf_add(msg_key_large.data + 8, 16);
		buf_free(msg_key_large);

		//sha256_a = SHA256 (msg_key + substr (auth_key, x, 36));
		buf_t sha256_a_ = 
			buf_add(msg_key.data, msg_key.size);
		sha256_a_ = 
			buf_cat_data(sha256_a_, tg->key.data, 36);
		buf_t sha256_a = tg_hsh_sha256(sha256_a_);
		buf_free(sha256_a_);

		//sha256_b = SHA256 (substr (auth_key, 40+x, 36) + msg_key);
		buf_t sha256_b_ = 
			buf_add(tg->key.data + 40, 36);
		sha256_b_ = 
			buf_cat_data(sha256_b_, msg_key.data, msg_key.size);
		buf_t sha256_b = tg_hsh_sha256(sha256_b_);
		buf_free(sha256_b_);

		//aes_key = substr (sha256_a, 0, 8) 
		//+ substr (sha256_b, 8, 16) + substr (sha256_a, 24, 8);
		buf_t aes_key = buf_add(sha256_a.data, 8);
		aes_key = 
			buf_cat_data(aes_key, sha256_b.data + 8, 16);
		aes_key = 
			buf_cat_data(aes_key, sha256_a.data + 24, 8);
		
		//aes_iv = substr (sha256_b, 0, 8) + 
		//substr (sha256_a, 8, 16) + substr (sha256_b, 24, 8);
		buf_t aes_iv = buf_add(sha256_b.data, 8);
		aes_iv = 
			buf_cat_data(aes_iv, sha256_a.data + 8, 16);
		aes_iv = 
			buf_cat_data(aes_iv, sha256_b.data + 24, 8);
		
		// Encrypted Message: encrypted_data
		buf_t enc = tg_cry_aes_e(b, aes_key, aes_iv);
		buf_free(aes_key);
		buf_free(aes_iv);
		buf_free(sha256_a);
		buf_free(sha256_b);
		
		// Encrypted Message
		// auth_key_id msg_key encrypted_data
		// int64       int128  bytes
		e = buf_cat(e, auth_key_id);
		e = buf_cat(e, msg_key);
		e = buf_cat(e, enc);
		buf_free(auth_key_id);
		buf_free(msg_key);
		buf_free(enc);

	}else {
		e = buf_cat(e, b);	
	}

	ON_LOG_BUF(tg, e, "%s: ", __func__);
  return e;
}

static buf_t decrypt(tg_t *tg, buf_t m, bool enc)
{
  buf_t d;
	buf_init(&d);

	if (!m.size) {
    ON_ERR(tg, NULL, "%s: received nothing", __func__);
		return d;
  }

  if (enc){
		// Encrypted Message
		// auth_key_id msg_key encrypted_data
		// int64       int128  bytes
			
		// auth_key
		uint64_t auth_key_id = buf_get_ui64(m);
		// check matching
		buf_t key_hash = tg_hsh_sha1(tg->key);
		buf_t auth_key_id_ = buf_add(key_hash.data + 12, 8);
		if (auth_key_id != buf_get_ui64(auth_key_id_)){
			ON_ERR(tg, NULL, "%s: auth_key_id mismatch", __func__);
			buf_free(auth_key_id_);
			return buf_cat(d, m);
		}
		buf_free(auth_key_id_);
		auth_key_id = deserialize_ui64(&m);

		// msg_key
		buf_t msg_key = deserialize_buf(&m, 16);
		
		// encrypted_data
		//sha256_a = SHA256 (msg_key + substr (auth_key, x, 36));
		buf_t sha256_a_ = 
			buf_add(msg_key.data, msg_key.size);
		sha256_a_ = 
			buf_cat_data(sha256_a_, tg->key.data + 8, 36);
		buf_t sha256_a = tg_hsh_sha256(sha256_a_);
		buf_free(sha256_a_);

		//sha256_b = SHA256 (substr (auth_key, 40+x, 36) + msg_key);
		buf_t sha256_b_ = 
			buf_add(tg->key.data + 40+8, 36);
		sha256_b_ = 
			buf_cat(sha256_b_, msg_key);
		buf_t sha256_b = tg_hsh_sha256(sha256_b_);
		buf_free(sha256_b_);
		
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
		
		d = tg_cry_aes_d(m, aes_key, aes_iv);

	} else{
		d = buf_cat(d, m);
	}

	ON_LOG_BUF(tg, d, "%s: ", __func__);
  return d;
}


static buf_t transport(tg_t *tg, buf_t buf)
{
  buf_t b;
	buf_init(&b);
	
	// intermediate header
	b = buf_cat_ui32(b, buf.size);
	b = buf_cat(b, buf);

	ON_LOG_BUF(tg, b, "%s: ", __func__);
  return b;
}

static buf_t detransport(tg_t *tg, buf_t a)
{
	buf_t b;
	buf_init(&b);
	
  if (!a.size) {
    ON_ERR(tg, NULL, "%s: received nothing", __func__);
		return b;
  }

	uint32_t len = deserialize_ui32(&a);
	b = buf_cat(b, a);

	if (a.size == 4 && buf_get_ui32(a) == 0xfffffe6c) {
    ON_ERR(tg, NULL, "%s: 404", __func__);
		return b;
	}

	// check len
	if (len != b.size) {
		ON_ERR(tg, NULL, 
				"%s: len mismatch: expected: %d, got: %d", 
				__func__, len, b.size);
	}

  return b;
}

tl_t * tg_handle_serialized_message(tg_t *tg, buf_t msg);

tl_t * tg_handle_deserialized_message(tg_t *tg, tl_t *tl)
{
	int i;
	switch (tl->_id) {
		case id_msg_container:
			{
				tl_msg_container_t *obj = 
					(tl_msg_container_t *)tl;
				for (i = 0; i < obj->messages_len; ++i) {
					mtp_message_t m = obj->messages_[i];
					tl = tg_handle_serialized_message(tg, m.body);	
				}
			}
			break;
		case id_new_session_created:
			{
				tl_new_session_created_t *obj = 
					(tl_new_session_created_t *)tl;
				// handle new session
			}
			break;
		case id_pong:
			{
				tl_pong_t *obj = 
					(tl_pong_t *)tl;
				// handle pong
			}
			break;
		case id_msgs_ack:
			{
				tl_msgs_ack_t *obj = 
					(tl_msgs_ack_t *)tl;
				// get new message
				buf_t r = tg_net_receive(tg);
				buf_t tr = detransport(tg, r);
				buf_free(r);
				if (buf_get_ui32(tr) == 0xfffffe6c){
					tl_t *tl = NEW(tl_t, return NULL);
					tl->_id = buf_get_ui32(tr);
					return tl;
				}
				buf_t d = decrypt(tg, tr, true);
				buf_free(tr);
				buf_t msg = deheader(tg, d, true);
				buf_free(d);
				tl = tg_handle_serialized_message(tg, msg);
			}
			break;
		case id_rpc_result:
			{
				tl_rpc_result_t *obj = 
					(tl_rpc_result_t *)tl;
				tl_t *result = 
					tg_handle_deserialized_message(tg, obj->result_);
				if (result){
					tl = result;
					// acknowlege
					/* TODO:  <12-11-24, kuzmich> */
				}
				
				if (result && result->_id == id_gzip_packed){
					// handle gzip
					tl_gzip_packed_t *obj =
						(tl_gzip_packed_t *)result;

					buf_t buf;
					int _e = gunzip_buf(&buf, obj->packed_data_);
					if (_e)
					{
						char *err = gunzip_buf_err(_e);
						ON_ERR(tg, result, "%s: %s", __func__, err);
						free(err);
					}
					tl = tg_handle_serialized_message(tg, buf);
				}
			}
			break;
		case id_bad_msg_notification:
			{
				tl_bad_msg_notification_t *obj = 
					(tl_bad_msg_notification_t *)tl;
				// handle bad msg notification
			}
			break;

		default:
			break;
	}

	return tl;
}

tl_t * tg_handle_serialized_message(tg_t *tg, buf_t msg)
{
	if (!msg.size)
		return NULL;

	tl_t *tl = tl_deserialize(&msg);
	if (!tl)
		return NULL;

	return tg_handle_deserialized_message(tg, tl);
}

tl_t * tg_send_query_(tg_t *tg, buf_t query, bool enc)
{
	if (!tg->sockfd)
		tg_net_open(tg);
	
	tg->seqn++;
	
	buf_t h = header(tg, query, enc);
	
	buf_t e = encrypt(tg, h, enc);
	buf_free(h);

	buf_t t = transport(tg, e);
	buf_free(e);

	tg_net_send(tg, t);
	buf_free(t);
	
	buf_t r = tg_net_receive(tg);

	buf_t tr = detransport(tg, r);
	if (!tr.size)
		return NULL;
	if (buf_get_ui32(tr) == 0xfffffe6c){
		tl_t *tl = NEW(tl_t, return NULL);
		tl->_id = 0xfffffe6c;
		return tl;
	}
	buf_free(r);

	buf_t d = decrypt(tg, tr, enc);
	if (!d.size)
		return NULL;
	buf_free(tr);

	buf_t msg = deheader(tg, d, enc);
	if (!msg.size)
		return NULL;
	buf_free(d);

	tl_t *tl = tg_handle_serialized_message(tg, msg);
	if (tl && tl->_id == id_bad_server_salt) // resend message
		tl = tg_send_query_(tg, query, enc);

	buf_free(msg);
	
	return tl;
}

tl_t * tg_send_query(tg_t *tg, buf_t s)
{
	return tg_send_query_(tg, s, true);
}
