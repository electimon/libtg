#include "deserialize.h"
#include "deserialize_table.h"
#include "tl_object.h"
#include <string.h>

static tl_deserialize_function *get_fun(unsigned int id){
	int i,
			len = sizeof(tl_deserialize_table)/
				    sizeof(*tl_deserialize_table);

	for (i = 0; i < len; ++i)
		if(tl_deserialize_table[i].id == id)
			return tl_deserialize_table[i].fun;

	return NULL;
}

tlo_t * tg_deserialize(buf_t *buf)
{
	ui32_t *id = (ui32_t *)(buf->data);

	// find id in deserialize table
	tl_deserialize_function *fun = get_fun(*id);
	if (!fun){
		printf("can't find deserialization"
				" function for id: %.8x\n", *id);
		return NULL;
	}

	// run deserialization function
	return fun(buf);
}
