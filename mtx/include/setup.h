//
//  setup.h
//  mtx
//
//  Created by Pavel Morozkin on 17.01.14.
//  Copyright (c) 2014 Pavel Morozkin. All rights reserved.
//

#ifndef mtx_setup_h
#define mtx_setup_h

#define product             "mtx"
#define version_            "0.1"

#define ip_                 "149.154.167.40"
//#define ip_                 "149.154.167.50"
#define port_               443

#define api_id_             24646404
#define api_hash_           "818803c99651e8b777c54998e6ded6a0"
#define lang_code_          "ru"

#define public_key          "pub.pkcs"

enum settings_atomic_operations
{
  max_buf_size              = 1024,
};

enum settings_tgt
{
  max_param_size            = 12,
  max_abstract_params       = 24,
  max_container_size        = 8,
};

enum settings_id_codes
{
  id_resPQ                  = 0x05162463,
  id_Vector                 = 0x1cb5c415,
  id_Server_DH_Params_ok    = 0xd0e8075c,
  id_server_DH_inner_data   = 0xb5890dba,
  id_dh_gen_ok              = 0x3bcbf734,
  id_msg_container          = 0x73f1f8dc,
  id_new_session_created    = 0x9ec20908,
  id_pong                   = 0x347773c5,
  id_bad_msg_notification   = 0xa7eff811,
  id_msgs_ack               = 0x62d6b459,
  id_rpc_result             = 0xf35c6d01,
	id_rpc_error              = 0x2144ca19,
	id_auth_sentCode          = 0x5e002502,
};

#endif
