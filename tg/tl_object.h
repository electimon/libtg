#ifndef TL_OBJECT_H_
#define TL_OBJECT_H_
#include "../mtx/include/tgt.h"
#include "../mtx/include/buf.h"

typedef struct tl_object_ {
	buf_t  __value;
	ui32_t __id;
	tl_type_t __type;
	char name[64];
	param_t params[32];
	int nparams;
} tlo_t;

#endif /* ifndef TL_OBJECT_H_ */
