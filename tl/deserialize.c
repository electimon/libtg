#include "tl.h"
#include "deserialize_table.h"
#include "../mtx/include/api.h"
#include <string.h>

buf_t deserialize_bytes(buf_t *b)
{
	buf_dump(*b);
  buf_t s;
  buf_t byte = buf_add(b->data, 4);
  int offset = byte.data[0];
  ui32_t len = 0;

  /*if (byte.data[0] <= 253 && !b.data[1 + offset] && !b.data[2 + offset] && !b.data[3 + offset]) {*/
  if (byte.data[0] <= 253) {
    len = byte.data[0];
    s = buf_add(b->data +1, len);
  } else if (byte.data[0] >= 254) {
    ui8_t start = 0xfe;
    buf_t s1 = buf_add((ui8_t *)&start, 1);
    buf_t s2 = buf_add(b->data, 1);

    if (!buf_cmp(s1, s2)) {
      api.log.error("can't deserialize bytes");
    }

    buf_t len_ = buf_add(b->data + 1, 3);
    len_.size = 4; // hack
    len = buf_get_ui32(len_);
    s = buf_add(b->data + 4, len);
  } else {
    api.log.error("can't deserialize bytes");
  }
	
	*b = buf_add(b->data + len, b->size - len);
  
	// padding
	int pad = (4 - (len % 4)) % 4;
	if (pad) {
		*b = buf_add(b->data + pad, b->size - pad);
	}

  return s;
}

static tl_deserialize_function *get_fun(unsigned int id){
	int i,
			len = sizeof(tl_deserialize_table)/
				    sizeof(*tl_deserialize_table);

	for (i = 0; i < len; ++i)
		if(tl_deserialize_table[i].id == id)
			return tl_deserialize_table[i].fun;

	return NULL;
}

tl_t * tl_deserialize(buf_t *buf)
{
	ui32_t *id = (ui32_t *)(buf->data);
	if (!*id)
		return NULL;

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
