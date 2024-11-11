#ifndef LIB_TL_H_
#define LIB_TL_H_
#include "tl.h"
#include "api_layer.h"
#include "id.h"
#include "names.h"
#include "struct.h"
#include "methods.h"
#include "deserialize.h"

ui32_t id_from_tl_buf(buf_t tl_buf);
char * string_from_buf(buf_t string);

/* send TL object to server and return answer */
tl_t *tl_send(buf_t tl_serialized_object);

void tl_handle_messages(tl_t *tl, void *userda, 
		int (*callback)(tl_t *tl, 
			ui64_t msg_id, ui32_t code, const char *msg));

#endif /* ifndef LIB_TL_H_ */ 
