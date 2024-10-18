#ifndef TL_OBJECT_H_
#define TL_OBJECT_H_
#include "../mtx/include/tgt.h"
#include "../mtx/include/buf.h"

typedef struct tl_object_ {
	ui32_t id;
	buf_t  value;
	tl_type_t type;
	char name[64];
	struct tl_object_ **objs;
	int nobjs;
	int nflags;
	int flag_num;
	int flag_bit;
} tlo_t;

void tl_object_free(tlo_t *obj);

#endif /* ifndef TL_OBJECT_H_ */
