#include "serialize.h"
#include "../mtx/include/sel.h"
#include <string.h>

buf_t tg_serialize(tlo_t *obj)
{
	buf_t buf = {};
	memset(&buf, 0, sizeof(buf_t));
	
	// set id
	buf_add_ui32(obj->id);

	int i;
	// collect flags
	int flags[32], nflags = 0;
	for (i = 0; i < obj->nobjs; ++i) {
		if (obj->objs[i]->type == TYPE_FLAG)
			flags[nflags++] = i;
	}

	// append params
	for (i = 0; i < obj->nobjs; ++i) {
		// check if has flag
		int flagn = obj->objs[i]->flag_num;
		if (flagn){
			// skip value if flag 0
			int flagb = (1 << obj->objs[i]->flag_bit);
			if ((flags[flagn] & flagb) != flagb)
				continue;
		}
		
		switch (obj->objs[i]->type) {
			case TYPE_INT: case TYPE_LONG: case TYPE_X:
				{
					buf_cat(buf, obj->objs[i]->value);
					break;	
				}
			case TYPE_BYTES:
				{
					buf_cat(buf, obj->objs[i]->value);
				}
			case TYPE_VECTOR:
				{
					buf_t b = buf_add_ui32(id_Vector);
					buf_cat(buf, b);
					buf_cat(buf, obj->objs[i]->value);
					break;
				}
			case TYPE_STRING:
				{
					buf_t b = 
						sel_serialize_string(obj->objs[i]->value);
					buf_cat(buf, b);

					break;
				}
				
			default:
				break;
		}	
	}

	return buf;
}
