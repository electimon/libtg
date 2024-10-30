#ifndef SYMBOL
#define SYMBOL

#include  "tl/libtl.h"

// LibTG structure
typedef struct tg_ tg_t;

/* create new libtg structure */
tg_t * tg_new(
		const char *database_path, 
		int apiId, const char apiHash[33]);

/* free libtg structure and free memory */
void tg_close(tg_t *tg);

/* send TL object to server and return answer */
buf_t tg_send(tg_t *tg, buf_t tl_serialized_object);

typedef enum {
	TG_AUTH_SUCCESS,
	TG_AUTH_SENDCODE,
	TG_AUTH_PASSWORD_NEEDED,
	TG_AUTH_ERROR,
} TG_AUTH;

/* connect to Telegram */  
int tg_connect(
		tg_t *tg,
		const char *phone_number,
		void *userdata,
		char * (*callback)(
			void *userdata,
			TG_AUTH auth,
			tl_user_t *user));

#endif /* ifndef SYMBOL */
