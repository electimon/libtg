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

/* send TL object to server and return answer */
buf_t tl_send(buf_t tl_serialized_object);

#endif /* ifndef LIB_TL_H_ */ 
