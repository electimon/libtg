//
//  tgt.h
//  mtx
//
//  Created by Pavel Morozkin on 18.01.14.
//  Copyright (c) 2014 Pavel Morozkin. All rights reserved.
//

#ifndef __mtx__tgt__
#define __mtx__tgt__

#include "types.h"
#include "macro.h"

typedef enum msg_
{
  API,
  RFC,
  CTER,
} msg_t;

#define BOOL_FALSE 0xbc799737
#define BOOL_TRUE  0x997275b5

typedef enum tl_type_
{
  TYPE_QX,
  TYPE_BOOL,
  TYPE_CHAT,
  TYPE_CHATFULL,
  TYPE_CHATPARTICIPANTS,
  TYPE_CHATPHOTO,
  TYPE_FILELOCATION,
  TYPE_GEOPOINT,
  TYPE_INPUTCHATPHOTO,
  TYPE_INPUTFILE,
  TYPE_INPUTFILELOCATION,
  TYPE_INPUTGEOPOINT,
  TYPE_INPUTMEDIA,
  TYPE_INPUTNOTIFYPEER,
  TYPE_INPUTPEER,
  TYPE_INPUTPEERNOTIFYSETTINGS,
  TYPE_INPUTPHOTO,
  TYPE_INPUTPHOTOCROP,
  TYPE_INPUTUSER,
  TYPE_INPUTVIDEO,
  TYPE_MESSAGE,
  TYPE_MESSAGEACTION,
  TYPE_MESSAGEMEDIA,
  TYPE_MESSAGESFILTER,
  TYPE_PEER,
  TYPE_PEERNOTIFYSETTINGS,
  TYPE_PHOTO,
  TYPE_PHOTOSIZE,
  TYPE_UPDATE,
  TYPE_USER,
  TYPE_USERPROFILEPHOTO,
  TYPE_USERSTATUS,
  TYPE_VECTOR_CHAT,
  TYPE_VECTOR_CHATPARTICIPANT,
  TYPE_VECTOR_CONTACT,
  TYPE_VECTOR_CONTACTBLOCKED,
  TYPE_VECTOR_CONTACTFOUND,
  TYPE_VECTOR_CONTACTSUGGESTED,
  TYPE_VECTOR_DCOPTION,
  TYPE_VECTOR_DIALOG,
  TYPE_VECTOR_IMPORTEDCONTACT,
  TYPE_VECTOR_INPUTAPPEVENT,
  TYPE_VECTOR_INPUTCONTACT,
  TYPE_VECTOR_INPUTUSER,
  TYPE_VECTOR_MESSAGE,
  TYPE_VECTOR_PHOTO,
  TYPE_VECTOR_PHOTOSIZE,
  TYPE_VECTOR_UPDATE,
  TYPE_VECTOR_USER,
  TYPE_VECTOR_INT,
  TYPE_VECTOR_LONG,
  TYPE_VECTOR_STRING,
  TYPE_VECTOR,
  TYPE_VIDEO,
  TYPE_BYTES,
  TYPE_CONTACTS_FOREIGNLINK,
  TYPE_CONTACTS_LINK,
  TYPE_CONTACTS_MYLINK,
  TYPE_DOUBLE,
  TYPE_INT,
  TYPE_INT128,
  TYPE_INT256,
  TYPE_LONG,
  TYPE_STRING,
  TYPE_UPDATES_STATE,
  TYPE_ID,
  TYPE_X,
  TYPE_FLAG,
} tl_type_t;

typedef struct param_
{
  ui32_t        id;
  buf_t         value;
  tl_type_t     type;
	int           flag_num;
	int           flag_bit;
} param_t;

typedef struct abstract_
{
  param_t       params[max_abstract_params];
  ui32_t        size;
  msg_t         type;
  stk_mode_t    stk_mode;
} abstract_t;

typedef void method_callback_t(
		void *userdata, abstract_t *a);

typedef struct method_req_pq_ method_req_pq_t;

typedef struct method_req_DH_params_ method_req_DH_params_t;

typedef struct method_set_client_DH_params_ method_set_client_DH_params_t;

typedef struct method_ping_ method_ping_t;

typedef struct method_auth_sendCode_ method_auth_sendCode_t;

typedef struct method_msgs_ack_ method_msgs_ack_t;

typedef struct ctor_ResPQ_ ctor_ResPQ_t;

typedef struct ctor_Server_DH_Params_ ctor_Server_DH_Params_t;

typedef struct ctor_Server_DH_Params_ok_ ctor_Server_DH_Params_ok_t;

typedef struct ctor_Server_DH_Params_fail_ ctor_Server_DH_Params_fail_t;

typedef struct ctor_P_Q_inner_data_ ctor_P_Q_inner_data_t;

typedef struct ctor_Server_DH_inner_data_ ctor_Server_DH_inner_data_t;

typedef struct ctor_Client_DH_Inner_Data_ ctor_Client_DH_Inner_Data_t;

typedef struct ctor_Set_client_DH_params_answer_ ctor_Set_client_DH_params_answer_t;

typedef struct ctor_Set_client_DH_params_answer_ok_ ctor_Set_client_DH_params_answer_ok_t;

typedef struct ctor_Set_client_DH_params_answer_retry_ ctor_Set_client_DH_params_answer_retry_t;

typedef struct ctor_Set_client_DH_params_answer_fail_ ctor_Set_client_DH_params_answer_fail_t;

typedef struct ctor_Pong_ ctor_Pong_t;

typedef struct ctor_auth_SentCode_ ctor_auth_SentCode_t;

typedef struct ctor_NewSession_ ctor_NewSession_t;

struct ctor_ResPQ_
{
  msg_t         type__;
  ui32_t        id__;
  param_t       nonce;
  param_t       server_nonce;
  param_t       pq;
  param_t       server_public_key_fingerprints;
};

struct method_req_pq_
{
  msg_t         type__;
  ui32_t        id__;
  param_t       nonce;
  ctor_ResPQ_t  ctor_ResPQ;
};

method_req_pq_t method_req_pq_init();

method_req_pq_t method_req_pq_drive(method_req_pq_t m);

struct ctor_P_Q_inner_data_
{
  msg_t         type__;
  ui32_t        id__;
  param_t       pq;
  param_t       p;
  param_t       q;
  param_t       nonce;
  param_t       server_nonce;
  param_t       new_nonce;
};

ctor_P_Q_inner_data_t ctor_P_Q_inner_data_init(method_req_pq_t m1, method_req_DH_params_t m2);

buf_t ctor_P_Q_inner_data_drive(ctor_P_Q_inner_data_t c);

struct ctor_Server_DH_Params_ok_
{
  msg_t         type__;
  ui32_t        id__;
  param_t       nonce;
  param_t       server_nonce;
  param_t       encrypted_answer;
};

struct ctor_Server_DH_Params_fail_
{
  msg_t         type__;
  ui32_t        id__;
  param_t       nonce;
  param_t       server_nonce;
  param_t       new_nonce_hash;
};

struct ctor_Server_DH_Params_
{
  ctor_Server_DH_Params_ok_t ctor_Server_DH_Params_ok;
  ctor_Server_DH_Params_fail_t ctor_Server_DH_Params_fail;
};

struct method_req_DH_params_
{
  msg_t         type__;
  ui32_t        id__;
  param_t       nonce;
  param_t       server_nonce;
  param_t       p;
  param_t       q;
  param_t       public_key_fingerprint;
  param_t       encrypted_data;

  ctor_Server_DH_Params_t   ctor_Server_DH_Params;
  ctor_P_Q_inner_data_t     ctor_P_Q_inner_data;
};

method_req_DH_params_t method_req_DH_params_init(method_req_pq_t m1);

method_req_DH_params_t method_req_DH_params_drive(method_req_DH_params_t m);

struct ctor_Server_DH_inner_data_
{
  msg_t         type__;
  ui32_t        id__;
  param_t       nonce;
  param_t       server_nonce;
  param_t       g;
  param_t       dh_prime;
  param_t       g_a;
  param_t       server_time;
  buf_t         answer;
  buf_t         tmp_aes_key;
  buf_t         tmp_aes_iv;
};

ctor_Server_DH_inner_data_t
ctor_Server_DH_inner_data_init(method_req_pq_t, method_req_DH_params_t);

ctor_Server_DH_inner_data_t
ctor_Server_DH_inner_data_drive(buf_t);

struct ctor_Client_DH_Inner_Data_
{
  msg_t         type__;
  ui32_t        id__;
  param_t       nonce;
  param_t       server_nonce;
  param_t       retry_id;
  param_t       g_b;
  buf_t         b;
};

ctor_Client_DH_Inner_Data_t
ctor_Client_DH_Inner_Data_init(method_req_pq_t m1, ctor_Server_DH_inner_data_t c1);

buf_t ctor_Client_DH_Inner_Data_drive(ctor_Client_DH_Inner_Data_t);

struct ctor_Set_client_DH_params_answer_ok_
{
  msg_t         type__;
  ui32_t        id__;
  param_t       nonce;
  param_t       server_nonce;
  param_t       new_nonce_hash1;
};

struct ctor_Set_client_DH_params_answer_retry_
{
  // ..
};

struct ctor_Set_client_DH_params_answer_fail_
{
  // ..
};

struct ctor_Set_client_DH_params_answer_
{
  ctor_Set_client_DH_params_answer_ok_t     ctor_Set_client_DH_params_answer_ok;
  ctor_Set_client_DH_params_answer_retry_t  ctor_Set_client_DH_params_answer_retry;
  ctor_Set_client_DH_params_answer_fail_t   ctor_Set_client_DH_params_answer_fail;
};

struct method_set_client_DH_params_
{
  msg_t         type__;
  ui32_t        id__;
  param_t       nonce;
  param_t       server_nonce;
  param_t       encrypted_data;

  ctor_Set_client_DH_params_answer_t  ctor_Set_client_DH_params_answer;
  ctor_Server_DH_inner_data_t         ctor_Server_DH_inner_data;
  ctor_Client_DH_Inner_Data_t         ctor_Client_DH_Inner_Data;

  buf_t         salt;
};

method_set_client_DH_params_t
method_set_client_DH_params_init(method_req_pq_t m1, method_req_DH_params_t m2);

method_set_client_DH_params_t
method_set_client_DH_params_drive(method_set_client_DH_params_t);

typedef struct ctor_MsgsAck_
{
  msg_t         type__;
  ui32_t        id__;
  param_t       msg_ids;
} ctor_MsgsAck_t;

struct ctor_Pong_
{
  msg_t         type__;
  ui32_t        id__;
  param_t       msg_id;
  param_t       ping_id;
};

typedef struct CodeSettings_
{
  msg_t         type__;
  ui32_t        id__;
  param_t       flags;
  param_t       allow_flashcall;
  param_t       current_number;
  param_t       allow_app_hash;
  param_t       allow_missed_call;
  param_t       allow_firebase;
  param_t       unknown_number;
  param_t       logout_tokens;
  param_t       device_token;
  param_t       is_app_sandbox;
} CodeSettings_t;

struct method_ping_
{
  msg_t         type__;
  ui32_t        id__;
  param_t       ping_id;
  ctor_Pong_t   ctor_Pong;
};

method_ping_t method_ping_init();

method_ping_t method_ping_drive(method_ping_t);

struct ctor_auth_SentCode_
{
  msg_t         type__;
  ui32_t        id__;
	param_t       flags;
	param_t       type;
  param_t       phone_code_hash;
  param_t       next_type;
  param_t       timeout;
};

struct method_auth_sendCode_
{
  msg_t         type__;
  ui32_t        id__;
  param_t       phone_number;
	param_t       sms_type;
  param_t       api_id;
  param_t       api_hash;
	param_t       lang_code;
	param_t       settings;
	/*ctor_auth_SentCode_t  ctor_auth_SentCode;*/
};

method_auth_sendCode_t method_auth_sendCode_init(
		const char *phone_number);

ctor_auth_SentCode_t
method_auth_sendCode_drive(method_auth_sendCode_t);


typedef struct method_auth_resendCode_
{
  msg_t         type__;
  ui32_t        id__;
	param_t       flags;
  param_t       phone_number;
	param_t       phone_code_hash;
  param_t       reason;
} method_auth_resendCode_t;

method_auth_resendCode_t 
method_auth_resendCode_init(const char *phone_code_hash);

ctor_auth_SentCode_t
method_auth_resendCode_drive(method_auth_resendCode_t);

typedef struct method_auth_singIn_
{
  msg_t         type__;
  ui32_t        id__;
	param_t       flags;
  param_t       phone_number;
	param_t       phone_code_hash;
  param_t       phone_code;
  param_t       email_verification;
} method_auth_singIn_t;

method_auth_singIn_t 
method_auth_singIn_init(
		const char *phone_code_hash,
		const char *phone_code);

ctor_auth_SentCode_t
method_auth_singIn_drive(method_auth_singIn_t);

struct ctor_NewSession_
{
  msg_t         type__;
  ui32_t        id__;
  param_t       first_msg_id;
  param_t       unique_id;
  param_t       server_salt;
};

typedef struct ctor_MessageContainer_
{
  msg_t         type__;
  ui32_t        id__;
  param_t       messages;
} ctor_MessageContainer_t;

struct method_msgs_ack_
{
  msg_t         type__;
  ui32_t        id__;
  param_t       msg_ids;
  param_t       ctor_MsgsAck;
};

method_msgs_ack_t method_msgs_ack_init(buf_t msg_id);

method_msgs_ack_t method_msgs_ack_drive(method_msgs_ack_t);

typedef struct ctor_rpc_error_
{
  msg_t         type__;
  ui32_t        id__;
  param_t       error_code;
  param_t       error_message;
} ctor_rpc_error_t;

typedef struct method_initConnection_
{
  msg_t         type__;
  ui32_t        id__;
	param_t       flags; //#Flags, see TL conditional fields
	param_t       api_id; //int Application identifier 
												//(see. App configuration)
	param_t device_model;//     string
											 //     Device model
	param_t system_version;//   string
												 //   Operation system version
	param_t app_version;//      string
											//      Application version
	param_t system_lang_code;// string                   Code
													 // for the language used on the
													 // device's OS, ISO 639-1
													 // standard
	param_t lang_pack;//        string
										//        Platform identifier (i.e.
										//        android, tdesktop, etc).
	param_t lang_code;//        string
										//        Either an ISO 639-1 language
										//        code or a language pack name
										//        obtained from a language pack
										//        link.
	param_t proxy;//            flags.0?InputClientProxy Info
								//            about an MTProto proxy
	param_t params;//           flags.1?JSONValue
								 //           Additional initConnection
								 //           parameters.
								 // For now, only the tz_offset field is
								 // supported, for specifying the timezone
								 // offset in seconds.
	param_t query; //            !X                       The
								 //            query itself

} method_initConnection_t;

method_initConnection_t 
method_initConnection_init(buf_t query);

abstract_t
method_initConnection_drive(method_initConnection_t);

typedef struct method_invokeWithLayer_
{
  msg_t         type__;
  ui32_t        id__;
	param_t       layer;
	param_t       query;
} method_invokeWithLayer_t;

method_invokeWithLayer_t 
method_invokeWithLayer_init(int layer, buf_t query);

abstract_t
method_invokeWithLayer_drive(method_invokeWithLayer_t);

typedef struct tg_api_type_system_
{
  method_req_pq_t                     method_req_pq;
  method_req_DH_params_t              method_req_DH_params;
  method_set_client_DH_params_t       method_set_client_DH_params;
  method_ping_t                       method_ping;
  method_auth_sendCode_t              method_auth_sendCode;
  method_auth_resendCode_t            method_auth_resendCode;
  method_auth_singIn_t                method_auth_singIn;
  method_msgs_ack_t                   method_msgs_ack;
  method_initConnection_t             method_initConnection;
  method_invokeWithLayer_t            method_invokeWithLayer;
  ctor_ResPQ_t                        ctor_ResPQ;
  ctor_Server_DH_Params_t             ctor_Server_DH_Params;
  ctor_P_Q_inner_data_t               ctor_P_Q_inner_data;
  ctor_Server_DH_inner_data_t         ctor_Server_DH_inner_data;
  ctor_Client_DH_Inner_Data_t         ctor_Client_DH_Inner_Data;
  ctor_Set_client_DH_params_answer_t  ctor_Set_client_DH_params_answer;
  ctor_Pong_t                         ctor_Pong;
  ctor_auth_SentCode_t                ctor_auth_SentCode;
  ctor_MessageContainer_t             ctor_MessageContainer;
  ctor_NewSession_t                   ctor_NewSession;
	CodeSettings_t                      CodeSettings;
	long msg_id;
} tg_api_type_system_t;


#endif /* defined(__mtx__tgt__) */
