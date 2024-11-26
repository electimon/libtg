#include "config.h"
#include <stdio.h>
#include "libtg.h"
#include "mtx/include/api.h"
#include "mtx/include/buf.h"
#include "mtx/include/types.h"
#include "tl/buf.h"
#include "tl/deserialize.h"
#include "tl/id.h"
#include <stdbool.h>
#include <string.h>
#include "tl/alloc.h"
#include "tl/libtl.h"
#include "tl/names.h"
#include <time.h>

#include "api_id.h"
#include "tl/tl.h"

/*
char * callback(
			void *userdata,
			TG_AUTH auth,
			const tl_t *tl, 
			const char *msg)
{
	switch (auth) {
		case TG_AUTH_PHONE_NUMBER_NEEDED:
			{
				char phone[32];
				printf("enter phone number (+7XXXXXXXXXX): \n");
				scanf("%s", phone);
				return strdup(phone);
			}
			break;
		case TG_AUTH_PHONE_CODE_NEEDED:
			{
				int code;
				printf("enter code: \n");
				scanf("%d", &code);
				printf("code: %d\n", code);
				char phone_code[32];
				sprintf(phone_code, "%d", code);
				return strdup(phone_code);
			}
			break;
		case TG_AUTH_PASSWORD_NEEDED:
			{
				char password[64];
				printf("enter password: \n");
				scanf("%s", password);
				printf("password: %s\n", password);
				return strdup(password);
			}
			break;
		case TG_AUTH_SUCCESS:
			{
				printf("Connected as ");
				tl_user_t *user = (tl_user_t *)tl;
				printf("%s (%s)!\n", 
						(char *)user->username_.data, 
						(char *)user->phone_.data);
			}
			break;
		case TG_AUTH_ERROR:
			{
				if (msg)
					printf("tg_connect error: %s\n", msg);
			}
			break;
		
		case TG_AUTH_INFO:
			{
				if (msg)
					printf("tg_connect info: %s\n", msg);
			}
			break;

		default:
			break;
	}

	return NULL;
}

static void on_err(void *d, tl_t *tl, const char *err)
{
	printf("ERR: %s\n", err);
}

static int md_callback(
		void *data, 
		tl_messages_dialogsSlice_t *dialogs, 
		const char *err)
{
	printf("dialogs: %d\n", dialogs->dialogs_len);
	printf("messages: %d\n", dialogs->messages_len);
	printf("chats: %d\n", dialogs->chats_len);
	//printf("MSG: %s\n", STRING_T_TO_STR(m->message_));
	//printf("message len: %d\n", m->message_);
	int i;
	for (i = 0; i < dialogs->messages_len; ++i) {
		tl_message_t *m = dialogs->messages_[i];
		printf("MSG: %s\n", m->message_.data);
	}

	printf("CHATS:\n");
	for (i = 0; i < dialogs->chats_len; ++i) {
		if (dialogs->chats_[i]->_id == id_channel){
			tl_channel_t *channel = 
				(tl_channel_t *)dialogs->chats_[i];
			printf("Channel %d: %s\n", i, channel->title_.data);
		}
		//tl_chat_t *chat = dialogs->chats_[i];
		//if (chat->_id == id_chat)
			//printf("CHAT %d: %s\n", i, chat->title_.data);
		//if (chat->_id == id_chatEmpty)
			//printf("chat empty\n");
		//printf("%.8x\n", chat->_id);
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int SETUP_API_ID(apiId)
	char * SETUP_API_HASH(apiHash)
	
	tg_t *tg = tg_new(
			"test.db", 
			apiId, 
			apiHash, "pub.pkcs");
	
	tg_connect(tg, NULL, callback);

	tg_get_dialogs(tg, 0, 6, 
			NULL, md_callback);

	tg_close(tg);
	return 0;
}
*/

void on_err(void *d, tl_t *tl, const char *err){
	printf("!!!ERR: %s\n", err);
}

void on_log(void *d, const char *msg){
	printf("%s\n", msg);
}

int main(int argc, char *argv[])
{
	printf("TGTEST\n");
	int apiId = 0;
	char *apiHash = NULL;

	SETUP_API_ID(apiId)
	SETUP_API_HASH(apiHash)
	
	tg_t *tg = tg_new(
			"test.db", 
			apiId, 
			apiHash, 
			"pub.pkcs");

	tg_set_on_error(tg, NULL, on_err);
	tg_set_on_log  (tg, NULL, on_log);

	//tg_new_auth_key2(tg);
	tg_new_auth_key(tg);
	return 0;
}
