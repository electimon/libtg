#include "deserialize.h"
#include "tl.h"
#include "methods.h"
#include "id.h"
#include "struct.h"
#include "../mtx/include/tgt.h"

buf_t tl_salt_new()
{
	// server_salt = substr(new_nonce, 0, 8) XOR substr(server_nonce, 0, 8)
	method_req_pq_t m = method_req_pq_init();
  m = method_req_pq_drive(m);
  method_req_DH_params_t m1 = method_req_DH_params_init(m);
  m1 = method_req_DH_params_drive(m1);
  method_set_client_DH_params_t m2 =
      method_set_client_DH_params_init(m, m1);
	m2 = method_set_client_DH_params_drive(m2);

	return m2.salt;
	
	/*buf_t nonce = buf_rand(16);*/
	/*buf_t reqPQ = tl_req_pq_multi(nonce);*/
	/*buf_t answ = tl_send_tl_message(reqPQ, RFC);*/
	/*tl_t *tl = tl_deserialize(&answ);*/
	/*if (tl->_id == id_resPQ){*/
		/*tl_resPQ_t *tl = tl;*/
		/*buf_t new_nonce_s = buf_add(nonce.data, 8);*/
		/*buf_t server_nonce_s = buf_add(tl->server_nonce_.data, 8);*/
		/*return buf_xor(new_nonce_s, server_nonce_s);*/
	/*}*/

	buf_t t = {};
	return t;
}
