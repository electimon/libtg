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

/* return allocated string with error from tl object */
char * tg_strerr(tl_t *tl);

/* return authorized user tl object or throw error */
tl_user_t * 
tg_is_authorized(tg_t *tg, void *on_err_data,
		void (*on_err)(void *on_err_data, tl_t *tl, const char *err));

/* send auth code to phone number and return sentCode 
 * tl object or thow error */
tl_auth_sentCode_t *
tg_auth_sendCode(tg_t *tg, const char *phone_number, 
		void *on_err_data,
		void (*on_err)(void *on_err_data, tl_t *tl, const char *err));

/* signin with phone number and code, return authorized 
 * user tl object or thow error */
tl_user_t *
tg_auth_signIn(tg_t *tg, tl_auth_sentCode_t *sentCode, 
		const char *phone_number, const char *phone_code, 
		void *on_err_data,
		void (*on_err)(void *on_err_data, tl_t *tl, const char *err));

/* all-in-one auth function */
typedef enum {
	TG_AUTH_ERROR,
	TG_AUTH_INFO,
	TG_AUTH_PHONE_NUMBER_NEEDED,
	TG_AUTH_PHONE_CODE_NEEDED,
	TG_AUTH_PASSWORD_NEEDED,
	TG_AUTH_SUCCESS,
} TG_AUTH;

/* connect to Telegram */  
int tg_connect(
		tg_t *tg,
		void *userdata,
		char * (*callback)(
			void *userdata,
			TG_AUTH auth,
			const tl_t *tl,
			const char *msg));

#endif /* ifndef SYMBOL */
