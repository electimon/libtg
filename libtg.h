#ifndef SYMBOL
#define SYMBOL

// TL object
typedef struct tl_object_ tlo_t;

// LibTG structure
typedef struct tg_ tg_t;

/* create new libtg structure */
tg_t * tg_new(const char *apiId, const char apiHash);

/* free libtg structure and free memory */
void tg_free(tg_t *tg);

/* send TL object to server and callback answer */
int tg_send(tg_t *tg, tlo_t *object, void *userdata,
		int (*callback)(void *userdata, tlo_t *answer, const char *error));

#endif /* ifndef SYMBOL */
