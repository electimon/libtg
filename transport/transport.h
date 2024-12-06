#ifndef TG_TRANSPORT_H
#define TG_TRANSPORT_H

#include "../tg/tg.h"

buf_t tg_encrypt    (tg_t *tg, buf_t b, bool enc);
buf_t tg_decrypt    (tg_t *tg, buf_t b, bool enc);
buf_t tg_header     (tg_t *tg, buf_t b, bool enc);
buf_t tg_deheader   (tg_t *tg, buf_t b, bool enc);
buf_t tg_transport  (tg_t *tg, buf_t b);
buf_t tg_detransport(tg_t *tg, buf_t b);

buf_t tg_serialize_query(tg_t *tg, buf_t query, bool enc);
tl_t * tg_handle_deserialized_message(tg_t *tg, tl_t *tl, int sockfd);
tl_t * tg_handle_serialized_message(tg_t *tg, buf_t msg, int sockfd);

#endif /* ifndef TG_TRANSPORT_H */
