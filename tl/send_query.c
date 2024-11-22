#define _POSIX_C_SOURCE 199309L
#include "../mtx/include/types.h"
#include "buf.h"
#include "../mtx/include/buf.h"
#include "../mtx/include/crc.h"
#include "hsh.h"
#include "deserialize.h"
#include "id.h"
#include "libtl.h"
#include "tl.h"
#include "types.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <zconf.h>
#include "zlib.h"
#include <stdlib.h>
#include "deserialize.h"
#include "alloc.h"
#include "net.h"
#include "cry.h"
#include <assert.h>
#include <time.h>
#include "../mtx/include/hdl.h"
#include "../mtx/include/enl.h"
#include "../mtx/include/trl.h"
#include "../mtx/include/net.h"

static void my_clock_gettime(int clock_id, struct timespec * T)
{
  assert(clock_gettime(clock_id, T) >= 0);
}

static double get_utime(int clock_id)
{
  struct timespec T;
  my_clock_gettime(clock_id, &T);
  double res = T.tv_sec + (double) T.tv_nsec * 1e-9;
  return res;
}

//static long long get_current_time()
//{
  //return (long long)((1LL << 32) * get_utime(CLOCK_REALTIME)) & -4;
//}

static buf_t header(buf_t b, bool enc)
{
  buf_t s = {};
	buf_init(&s);

  if (!enc)
    {
			//auth_key_id = 0 message_id message_data_length message_data
			//int64           int64      int32               bytes

			//auth_key_id
			s = buf_cat_ui64(s, 0);
			
			//message_id
      s = buf_cat_ui64(s, get_current_time());
			
			//message_data_length
      s = buf_cat_ui32(s, b.size);
			
			// message_data
      s = buf_cat(s, b);
    }
  else
    {
			// salt  session_id message_id seq_no message_data_length  message_data padding12..1024
			// int64 int64      int64      int32  int32                bytes        bytes
			
			// salt
      buf_t salt = 
				buf_add(shared_rc.salt.data, shared_rc.salt.size);
      s = buf_cat(s, salt);
			
			//session_id
      buf_t session_id = 
				buf_add(shared_rc.ssid.data, shared_rc.ssid.size);
      s = buf_cat(s, session_id);
			
			//message_id
      ui64_t msg_id_ = get_current_time();
      buf_t msg_id = buf_add((uint8_t *)&msg_id_, 8);
      s = buf_cat(s, msg_id);
			
			//seq_no
      ui32_t seqnh = shared_rc_get_seqnh();
      buf_t seq_no = buf_add_ui32(seqnh);
      s = buf_cat(s, seq_no);
			
			//message_data_length
      s = buf_cat_ui32(s, b.size);
			
			//message_data
      s = buf_cat(s, b);
			
			//padding
			uint32_t pad_ =  16 + (16 - (b.size % 16)) % 16;
			buf_t pad = buf_rand(pad_);
      s = buf_cat(s, pad);
			buf_free(pad);
		}

  return s;
}

static buf_t deheader(buf_t b, bool enc)
{
	if (!b.size){
    printf("got nothing\n");
		return b;
	}

  buf_t d;
	buf_init(&d);

  if (!enc)
    {
      // check msg_id diff (under construction...)
			uint64_t msg_id = buf_get_ui64(b);
      // remove msg_id
      d = buf_cat_data(d, b.data + 8, b.size - 8);
      
			/*ui32_t msg_data_len = buf_get_ui32(d);*/
      /*if (msg_data_len != d.size - 4) {*/
        /*printf("msg_data_len mismatch\n");*/
      /*}*/

      // remove len
      d = buf_add(d.data + 4, d.size - 4);
		}
  else 
    {
      // remove salt
			ui64_t salt = buf_get_ui64(b);
      shared_rc.salt = buf_add_ui64_(salt);
      d = buf_add(b.data + 8, b.size - 8);
      // remove session_id
      d = buf_add(d.data + 8, d.size - 8);
      // save last msg_id
      shared_rc.last_msg_id = buf_add_(d.data, 8);
      // remove message_id
      d = buf_add(d.data + 8, d.size - 8);
      // remove seq_no
      d = buf_add(d.data + 4, d.size - 4);
      ui32_t msg_data_len = buf_get_ui32(d);
      d.size -= 4;

      //if (msg_data_len != d.size) {
        //api.log.error("msg_data_len mismatch");
      //}
      
      // remove len
      d = buf_add(d.data + 4, d.size);
    }
    
  return d;
}


static buf_t encrypt(buf_t b, bool enc)
{
  buf_t e = {};
	buf_init(&e);

  if (enc) {
			buf_t auth_key = 
				buf_add(shared_rc.key.data, shared_rc.key.size);
			// For MTProto 2.0, SHA1 is still used here, because
			// auth_key_id should identify the authorization key
			// used independently of the protocol version.
      buf_t key_hash = tl_hsh_sha1(auth_key);
      buf_t auth_key_id = 
				buf_add(key_hash.data + 12, 8);
      
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
			buf_t msg_key_large = tl_hsh_sha256(msg_key_large_);

			// msg_key = substr (msg_key_large, 8, 16);
			buf_t msg_key = 
				buf_add(msg_key_large.data + 8, 16);

			//sha256_a = SHA256 (msg_key + substr (auth_key, x, 36));
			buf_t sha256_a_ = 
				buf_add(msg_key.data, msg_key.size);
			sha256_a_ = 
				buf_cat_data(sha256_a_, auth_key.data, 36);
			buf_t sha256_a = tl_hsh_sha256(sha256_a_);

			//sha256_b = SHA256 (substr (auth_key, 40+x, 36) + msg_key);
			buf_t sha256_b_ = 
				buf_add(auth_key.data + 40, 36);
			sha256_b_ = 
				buf_cat_data(sha256_b_, msg_key.data, msg_key.size);
			buf_t sha256_b = tl_hsh_sha256(sha256_b_);

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
      buf_t enc = tl_cry_aes_e(b, aes_key, aes_iv);
      
			// Encrypted Message
			// auth_key_id msg_key encrypted_data
			// int64       int128  bytes
      e = buf_cat(e, auth_key_id);
      e = buf_cat(e, msg_key);
			e = buf_cat(e, enc);

	}else {
		e = buf_cat(e, b);	
	}

  return e;
}

static buf_t decrypt(buf_t m, bool enc)
{
	/*printf("%s\n", __func__);*/
  buf_t d;
	buf_init(&d);

	if (!m.size) {
    printf("enl_decrypt: received nothing\n");
		return d;
  }

  if (enc)
    {
      buf_t auth_key = 
				buf_add(shared_rc.key.data, shared_rc.key.size);
      buf_t key_hash = tl_hsh_sha1(auth_key);
      buf_t auth_key_id  = buf_add(key_hash.data + 12, 8);
      buf_t auth_key_id_ = buf_add(m.data, 8);
      m = buf_add(m.data + 8, m.size - 8);

      if (!buf_cmp(auth_key_id, auth_key_id_)) {
        printf("different auth key id's\n");
      }

      buf_t msg_key = buf_add(m.data, 16);
      m = buf_add(m.data + 16, m.size - 16);
      //aes_params_t p = enl_get_params(key, msg_key, MODE_S2C);
			
			//sha256_a = SHA256 (msg_key + substr (auth_key, x, 36));
			buf_t sha256_a_ = 
				buf_add(msg_key.data, msg_key.size);
			sha256_a_ = 
				buf_cat_data(sha256_a_, auth_key.data + 8, 36);
			buf_t sha256_a = tl_hsh_sha256(sha256_a_);

			//sha256_b = SHA256 (substr (auth_key, 40+x, 36) + msg_key);
			buf_t sha256_b_ = 
				buf_add(auth_key.data + 40+8, 36);
			sha256_b_ = 
				buf_cat_data(sha256_b_, msg_key.data, msg_key.size);
			buf_t sha256_b = tl_hsh_sha256(sha256_b_);
			
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
			
			d = tl_cry_aes_d(m, aes_key, aes_iv);

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
        /*api.log.error("different msg key's\n");*/
      /*}*/
    
		} else
    {
      uint64_t key = buf_get_ui64(m);

      if (key != 0) {
        printf("trl_transport: keys mismatch\n");
      }

			// remove key
			d = buf_cat_data(d, m.data + 8, m.size - 8);
    }

	/*printf("%s done\n", __func__);*/
  return d;
}


static buf_t transport(buf_t buf)
{
  buf_t b = {};
	buf_init(&b);
	
	// intermediate header
	b = buf_cat_ui32(b, buf.size);
	b = buf_cat(b, buf);

  // add size
	//ui32_t len_ = buf.size + 12;
	//ui8_t * len_ptr = (ui8_t *)&len_;
	//buf_t len = buf_add(len_ptr, sizeof(buf.size));
	//b = buf_cat(b, len);
  // add seq
	//ui32_t seqn = shared_rc_get_seqn();
	//buf_t seq = buf_add_ui32(seqn);
	//b = buf_cat(b, seq);
  // add buf
	//b = buf_cat(b, buf);
  // add crc
	//buf_t_ b1 = buf_add_(b.data, b.size);
	//buf_t_ crc = crc_crc32(b1);
	//b = api.buf.cat(b, crc);
	//b = buf_cat_data(b, crc.data, crc.size);

  return b;
}

static buf_t detransport(buf_t a)
{
	buf_t b;

  if (!a.size) {
    printf("trl_transport: received nothing\n");
		return a;
  }

	ui32_t len = buf_get_ui32(a);
	b = buf_add(a.data + 4, len);

  ui32_t err = 0xfffffe6c;
	if (b.size == 4 && *(ui32_t *)(b.data) == err) {
		printf("trl_transport: 404\n");
		b.size = 0;
		return b;
	}

	// check len
	if (len != buf_get_ui32(b) + 4) {
		printf("trl_transport: len mismatch\n");
	}

  // check seq
	//buf_t a_seq = api.buf.add(a.data+4, 4);
  //if(!api.buf.cmp(seq, a_seq))
  //api.log.error("trl_transport: seq mismatch");

  // check crc
	//a.size -= 4;
	//buf_t a_crc = api.crc.crc32(a);
	//a.size += 4;
	//buf_t a_crc_ = api.buf.add(a.data + a.size - 4, 4);

  //if (!api.buf.cmp(a_crc, a_crc_)) {
    //api.log.error("trl_transport: crc mismatch");
  //}

	// remove
	//a.size -= 12;
	//a = api.buf.add(a.data + 8, a.size);
  
	//return a;
	return b;
}

tl_t * tl_handle_serialized_message(buf_t msg);

tl_t * tl_handle_deserialized_message(tl_t *tl)
{
	int i;
	switch (tl->_id) {
		case id_msg_container:
			{
				tl_msg_container_t *obj = 
					(tl_msg_container_t *)tl;
				for (i = 0; i < obj->messages_len; ++i) {
					mtp_message_t m = obj->messages_[i];
					tl = tl_handle_serialized_message(m.body);	
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
				buf_t r = tl_net_receive();
				buf_t tr = detransport(r);
				buf_t d = decrypt(tr, true);
				buf_t s1r = deheader(d, true);
				tl = tl_handle_serialized_message(s1r);
			}
			break;
		case id_rpc_result:
			{
				tl_rpc_result_t *obj = 
					(tl_rpc_result_t *)tl;
				tl_t *result = tl_handle_deserialized_message(obj->result_);
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
					if (gunzip_buf(&buf, obj->packed_data_))
						break;

					tl = tl_handle_serialized_message(buf);
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

tl_t * tl_handle_serialized_message(buf_t msg)
{
	if (!msg.size)
		return NULL;

	tl_t *tl = tl_deserialize(&msg);
	if (!tl)
		return NULL;

	return tl_handle_deserialized_message(tl);
}

tl_t * tl_send_query_(buf_t query, bool enc)
{
	shared_rc.seqnh++;
	/*printf("message:\n");*/
	/*buf_dump(s);*/

	//buf_t e;
	//buf_init(&e);
	
	/*  Encrypted Message
	 *
	 *  auth_key_id msg_key encrypted_data
	 *  int64       int128  bytes
	 *
	 *  Encrypted Message: encrypted_data
	 *
	 *  Contains the cypher text for the following data:
	 *
	 *  salt  session_id message_id seq_no message_data_length  message_data padding12..1024
	 *  int64 int64      int64      int32  int32                bytes        bytes
	 *
	 *  Unencrypted Message
	 *
	 *  auth_key_id = 0 message_id message_data_length message_data
	 *  int64           int64      int32               bytes
	 */


	//if (encrypt){
	
		//// auth key_id
    //buf_t key_hash = api.hsh.sha1(shared_rc.key);
    //buf_t auth_key_id = api.buf.add(key_hash.data + 12, 8);
		
		//// add salt and ssid
		//buf_t to_encrypt = 
			//buf_cat(shared_rc.salt, shared_rc.ssid);

		//// add message_id
		//buf_t msg_id = buf_add_ui64(get_current_time());
		//to_encrypt = buf_cat(to_encrypt, msg_id);	
      
		//// add seq_no
		//buf_t seq_no = buf_add_ui32(shared_rc.seqnh);
		//to_encrypt = buf_cat(to_encrypt, seq_no);	
		//free(seq_no.data);

		//// add data length
		//buf_t msg_data_len = buf_add_ui32(query.size);
		//to_encrypt = buf_cat(to_encrypt, msg_data_len);	

		////msg_key_large = SHA256 (substr (auth_key, 88+x, 32) 
		////+ plaintext + random_padding);
		
		//// add padding
		//ui32_t pad_ =  16 + (16 - (query.size % 16)) % 16;
		//buf_t pad = buf_rand(pad_);
		//buf_t data = buf_cat(query, pad);
		//free(pad.data);

		//buf_t msg_key_large_ = 
			//buf_add(shared_rc.key.data + 88, 32);
		//msg_key_large_ = buf_cat(msg_key_large_, data);
		//buf_t msg_key_large = hsh_sha256(msg_key_large_);

		//// msg_key = substr (msg_key_large, 8, 16);
		//buf_t msg_key = 
		//buf_add(msg_key_large.data + 8, 16);

		////sha256_a = SHA256 (msg_key + substr (auth_key, x, 36));
		//buf_t sha256_a_ = 
			//buf_add(msg_key.data, msg_key.size);
		//sha256_a_ = 
			//buf_cat_data(sha256_a_, shared_rc.key.data, 36);
		//buf_t sha256_a = hsh_sha256(sha256_a_);

		////sha256_b = SHA256 (substr (auth_key, 40+x, 36) + msg_key);
		//buf_t sha256_b_ = 
			//buf_add(shared_rc.key.data + 40, 36);
		//sha256_b_ = 
			//buf_cat_data(sha256_b_, msg_key.data, msg_key.size);
		//buf_t sha256_b = hsh_sha256(sha256_b_);

		////aes_key = substr (sha256_a, 0, 8) 
		////+ substr (sha256_b, 8, 16) + substr (sha256_a, 24, 8);
		//buf_t aes_key = buf_add(sha256_a.data, 8);
		//aes_key = buf_cat_data(aes_key, sha256_b.data + 8, 16);
		//aes_key = buf_cat_data(aes_key, sha256_a.data + 24, 8);
		
		////aes_iv = substr (sha256_b, 0, 8) + 
		////substr (sha256_a, 8, 16) + substr (sha256_b, 24, 8);
		//buf_t aes_iv = buf_add(sha256_b.data, 8);
		//aes_iv = buf_cat_data(aes_iv, sha256_a.data + 8, 16);
		//aes_iv = buf_cat_data(aes_iv, sha256_b.data + 24, 8);

		//// add data with padding
		//to_encrypt = buf_cat(to_encrypt, data);	

		//printf("to_encrypt\n");
		//buf_dump(to_encrypt);
		
		//// encrypt
    //buf_t enc = cry_aes_e(to_encrypt, aes_key, aes_iv);
//printf("%s: %d\n", __func__, __LINE__);

		//// make message
	 /*  auth_key_id msg_key encrypted_data
		*  int64       int128  bytes */
		//e = api.buf.cat(e, auth_key_id);
    //e = api.buf.cat(e, msg_key);
		//e = api.buf.cat(e, enc);
	
	//} else {
	 /*  auth_key_id = 0 message_id message_data_length message_data
		*  int64           int64      int32               bytes */
		
		//// add auth_key_id
		//buf_t auth_key_id = buf_add_ui64(0);
		//e = buf_cat(e, auth_key_id); // auth_key_id is 0
		
		//// add message_id
		//buf_t msg_id = buf_add_ui64(get_current_time());
		//e = buf_cat(e, msg_id);
		//free(msg_id.data);
	
		//// add size
		//buf_t size = buf_add_ui32(query.size);
    //e = api.buf.cat(e, size);
		//free(size.data);

		//// add query
    //e = api.buf.cat(e, query);

		//// free
	//}

	//// add transport
	//buf_t message = buf_add_ui32(e.size);
	//message = buf_cat(message, e);

	//// send message and receive answer
	//buf_t nr = api.net.drive(message, SEND_RECEIVE);
	//free(message.data);
	
	//printf("received:\n");
	//api.buf.dump(nr);

	//// get size
	//ui32_t size = deserialize_ui32(&nr);
	//if (size != nr.size){
		//printf("size differ!\n");
		//return NULL;
	//}

	//// check if 404 error
	//ui32_t id = buf_get_ui32(nr);
	//if (id == 0xfffffe6c){
		//printf("error 404\n");
		//tl_t *tl = NEW(tl_t, return NULL); 
		//tl->_id = id;
		//return tl;
	//}

	//// decrypt
	//buf_t d;
	//if (encrypt) {
		//buf_t auth_key = shared_rc_get_key();
		//buf_t key_hash = api.hsh.sha1(auth_key);
		//buf_t auth_key_id  = api.buf.add(key_hash.data + 12, 8);
		//buf_t auth_key_id_ = api.buf.add(nr.data, 8);
		//nr = api.buf.add(nr.data + 8, nr.size - 8);

		//if (!api.buf.cmp(auth_key_id, auth_key_id_)) {
			//api.log.error("different auth key id's");
		//}

		//buf_t msg_key = api.buf.add(nr.data, 16);
		//nr = api.buf.add(nr.data + 16, nr.size - 16);
		////aes_params_t p = enl_get_params(key, msg_key, MODE_S2C);
		
		////sha256_a = SHA256 (msg_key + substr (auth_key, x, 36));
		//buf_t sha256_a_ = 
			//buf_add(msg_key.data, msg_key.size);
		//sha256_a_ = 
			//buf_cat_data(sha256_a_, auth_key.data + 8, 36);
		//buf_t sha256_a = hsh_sha256(sha256_a_);

		////sha256_b = SHA256 (substr (auth_key, 40+x, 36) + msg_key);
		//buf_t sha256_b_ = 
			//buf_add(auth_key.data + 40+8, 36);
		//sha256_b_ = 
			//buf_cat_data(sha256_b_, msg_key.data, msg_key.size);
		//buf_t sha256_b = hsh_sha256(sha256_b_);
		
		////aes_key = substr (sha256_a, 0, 8) 
		////+ substr (sha256_b, 8, 16) + substr (sha256_a, 24, 8);
		//buf_t aes_key = buf_add(sha256_a.data, 8);
		//aes_key = buf_cat_data(aes_key, sha256_b.data + 8, 16);
		//aes_key = buf_cat_data(aes_key, sha256_a.data + 24, 8);
		
		////aes_iv = substr (sha256_b, 0, 8) + 
		////substr (sha256_a, 8, 16) + substr (sha256_b, 24, 8);
		//buf_t aes_iv = buf_add(sha256_b.data, 8);
		//aes_iv = buf_cat_data(aes_iv, sha256_a.data + 8, 16);
		//aes_iv = buf_cat_data(aes_iv, sha256_b.data + 24, 8);
		
		//d = api.cry.aes_d(nr, aes_key, aes_iv);
	//} else {
		//d = buf_add(nr.data + 8, nr.size - 8);
	//}

	//// remove header
	//buf_t response;
	//if (!encrypt){
		//// remove msg_id
		//ui64_t msg_id = deserialize_ui64(&d);
		//// save last msg_id
		//shared_rc.last_msg_id = buf_add_ui64(msg_id);

		//// msg_data_len
		//ui32_t msg_data_len = deserialize_ui32(&d);
		
		//// check data length
		//if (msg_data_len != d.size) {
			//api.log.error("msg_data_len mismatch");
		//}

		//response = api.buf.add(d.data, d.size);
	
	//} else {
		//// remove salt
		//ui64_t salt = deserialize_ui64(&d);
		//shared_rc.salt = buf_add_ui64(salt);
		
		//// remove session_id
		//ui64_t ssid = deserialize_ui64(&d);
		//// check session_id
		//if (ssid != buf_get_ui64(shared_rc.ssid)) {
			//api.log.error("ssid mismatch");
		//}
		
		//// remove message_id
		//ui64_t msg_id = deserialize_ui64(&d);

		//// save last msg_id
		//shared_rc.last_msg_id = buf_add_ui64(msg_id);

		//// remove seq_no
		//ui32_t seq_no = deserialize_ui32(&d);
		
		//// msg_data_len
		//ui32_t msg_data_len = deserialize_ui32(&d);
		
		//// check data length
		//if (msg_data_len != d.size) {
			//api.log.error("msg_data_len mismatch");
		//}

		//response = api.buf.add(d.data, d.size);
	//}
	
	//buf_t_ q = buf_add_(query.data, query.size);
	
	//buf_t_ s1 = hdl_header(q, API);
	//printf("header:\n");
	//buf_dump_(s1);

  //buf_t_ e = enl_encrypt(s1, API);
	//printf("encrypt:\n");
	//buf_dump_(e);
  
	//buf_t_ t = trl_transport(e);
	//printf("transport:\n");
	//buf_dump_(t);
  
	//buf_t_ nr = net_drive(t, SEND_RECEIVE);
	//printf("response:\n");
	//buf_dump_(nr);

	//buf_t_ tr = trl_detransport(nr);
  //buf_dump_(tr);
  
	//buf_t_ d = enl_decrypt(tr, API);
  //buf_dump_(d);
  
	//buf_t_ s1r = hdl_deheader(d, API);
  //buf_dump_(s1r);

	buf_t h = header(query, enc);
	printf("HEADER\n");
	//buf_t_ h = hdl_header(q, enc);
	buf_dump(h);
	
	buf_t e = encrypt(h, enc);
	printf("ENCRYPT\n");
	//buf_t_ e = enl_encrypt(h, enc);
	buf_dump(e);

	buf_t t = transport(e);
	printf("TRANSPORT\n");
	//buf_t_ t = trl_transport(e);
	buf_dump(t);

	tl_net_send(t);
	
	//buf_t_ n =  net_drive(t, SEND_RECEIVE);
	
	buf_t r = tl_net_receive();
	printf("NET:\n");
	buf_dump(r);

	buf_t tr = detransport(r);
	//buf_t_ tr = trl_detransport(n);
	printf("DETRANSPORT:\n");
	buf_dump(tr);

	buf_t d = decrypt(tr, enc);
	//buf_t_ d = enl_decrypt(tr, enc);
	printf("DECRYPT:\n");
	buf_dump(d);

	buf_t s1r = deheader(d, enc);
	//buf_t_ s1r = hdl_deheader(d, enc);
	printf("DEHEADER:\n");
	buf_dump(s1r);

	//buf_t buf = buf_add(s1r.data, s1r.size);

	tl_t *tl = tl_handle_serialized_message(s1r);
	if (tl && tl->_id == id_bad_server_salt) // resend message
		tl = tl_send_query_(query, enc);

	return tl;
}

tl_t * tl_send_query(buf_t s)
{
	return tl_send_query_(s, true);
}
