#ifndef TL_H_
#define TL_H_
#include "../mtx/include/buf.h"
#include <stdio.h>

typedef struct tl_ {
	ui32_t _id;
} tl_t;

typedef struct mtp_message_ {
	long msg_id;
	int seqno;
	int bytes;
	char body[BUFSIZ];
} mtp_message_t; 

#endif /* ifndef TL_H_ */
