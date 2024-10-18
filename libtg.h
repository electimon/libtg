#ifndef SYMBOL
#define SYMBOL

#include  "tg/methods.h"

// TL object
typedef struct tl_object_ tlo_t;

// LibTG structure
typedef struct tg_ tg_t;

/* create new libtg structure */
tg_t * tg_new();

/* free libtg structure and free memory */
void tg_free(tg_t *tg);

/* send TL object to server and callback answer */
tlo_t * tg_send(tg_t *tg, tlo_t *object);

#endif /* ifndef SYMBOL */
