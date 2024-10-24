#include "serialize.h"
#include "../mtx/include/sel.h"
#include <string.h>
#include <stdbool.h>

static bool flag_is_set(int value, int flag)
{
	int flagb = (1 << flag);
	return (value & flagb) != flagb;
}

buf_t tg_serialize(tlo_t *obj)
{
	buf_t buf = {};
	memset(&buf, 0, sizeof(buf_t));
	
	// set id
	printf("serialize object with id: %.8x\n", obj->id);
	buf = buf_add_ui32(obj->id);

	int i;
	// collect flags
	int flags[32], nflags = 0;
	for (i = 0; i < obj->nobjs; ++i) {
		if (obj->objs[i] && obj->objs[i]->type == TYPE_FLAG)
			flags[nflags++] = i;
	}
	nflags = 0;

	// append params
	for (i = 0; i < obj->nobjs; ++i) {
		// skip NULL obj
		if (obj->objs[i] == NULL)
			continue;
		// check if has flag
		int flagn = obj->objs[i]->flag_num;
		if (flagn){
			// skip value if flag 0
			if (flag_is_set(
						flags[flagn],
					 	obj->objs[i]->flag_bit)
					)
				continue;
		}
		/*printf("OBJECT TYPE: %d\n", obj->objs[i]->type);*/
	
		switch (obj->objs[i]->type) {
			case TYPE_FLAG:
				{
					//printf("FLAG!!!\n");
					buf_t b = buf_add_ui32(flags[nflags++]);
					buf = buf_cat(buf, b);
					break;
				}
			case TYPE_X:
				{
					//printf("X!!!\n");
					buf = buf_cat(buf, tg_serialize(obj->objs[i]));
					break;	
				}
			case TYPE_INT: case TYPE_LONG:
				{
					buf = buf_cat(buf, obj->objs[i]->value);
					break;	
				}
			case TYPE_BYTES:
				{
					/*buf = buf_cat(buf, obj->objs[i]->value);*/
					break;
				}
			case TYPE_VECTOR:
				{
					/*buf_t b = buf_add_ui32(id_Vector);*/
					/*buf = buf_cat(buf, b);*/
					/*buf = buf_cat(buf, tg_serialize(obj->objs[i]));*/
					break;
				}
			case TYPE_STRING:
				{
					buf_t b = 
						sel_serialize_string(obj->objs[i]->value);
					buf = buf_cat(buf, b);

					break;
				}
				
			default:
				break;
		}	
	}

	return buf;
}
