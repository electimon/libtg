#ifndef TL_DESERIALIZE_H
#define TL_DESERIALIZE_H
#include "tl.h"
ui32_t deserialize_ui32(buf_t *b);
ui32_t deserialize_ui64(buf_t *b);
buf_t deserialize_bytes(buf_t *b);
#define deserialize_string(b) deserialize_bytes(b)
mtp_message_t deserialize_mtp_message(buf_t *b);
tl_t * tl_deserialize(buf_t *buf);
#endif /* ifndef TL_DESERIALIZE_H */
