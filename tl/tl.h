#ifndef TL_H_
#define TL_H_
#include "buf.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

typedef struct tl_ {
	uint32_t _id;
} tl_t;

typedef struct mtp_message_ {
	long msg_id;
	int seqno;
	int bytes;
	buf_t body;
} mtp_message_t; 

#endif /* ifndef TL_H_ */
