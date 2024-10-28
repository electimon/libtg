#ifndef TL_DESERIALIZE_H
#define TL_DESERIALIZE_H
#include "tl.h"
buf_t deserialize_bytes(buf_t *b);
#define deserialize_string(b) deserialize_bytes(b)
tl_t * tl_deserialize(buf_t *buf);
#endif /* ifndef TL_DESERIALIZE_H */
