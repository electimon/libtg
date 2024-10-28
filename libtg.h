#ifndef SYMBOL
#define SYMBOL

#include  "tl/libtl.h"

// LibTG structure
typedef struct tg_ tg_t;

/* create new libtg structure */
tg_t * tg_new(
		const char *database_path, 
		int apiId, const char apiHash[32]);

/* free libtg structure and free memory */
void tg_close(tg_t *tg);

/* send TL object to server and return answer */
tl_t * tg_send(tg_t *tg, buf_t tl_serialized_object);

typedef enum {
	TG_AUTH_SUCCESS,
	TG_AUTH_SENDCODE,
	TG_AUTH_PASSWORD_NEEDED,
	TG_AUTH_ERROR,
} TG_AUTH;

/* connect to Telegram */  
int tg_connect(
		const char *phone_number,
		void *userdata,
		int (*callback)(
			void *userdata,
			TG_AUTH auth,
			User *user));

#endif /* ifndef SYMBOL */
