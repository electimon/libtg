#include "tl.h"
#include "../mtx/include/sel.h"
#include <string.h>
#include <stdbool.h>

buf_t serialize_bytes(
		ui8_t *bytes, size_t size)
{
	buf_t s;
	memset(&s, 0, sizeof(buf_t));
	if (size <= 253){
		s = api.buf.add((ui8_t *)&size, 1);
    buf_t b = buf_add(bytes, size);
    s = api.buf.cat(s, b);
    int pad = (4 - (s.size % 4)) % 4;
    buf_t p = {};
    p.size = pad;
    s = api.buf.cat(s, p);
	} else {
		ui8_t start = 0xfe;
    s = api.buf.add((ui8_t *)&start, 1);
    buf_t len = api.buf.add((ui8_t *)&size, 3);
    s = api.buf.cat(s, len);
    buf_t b = buf_add(bytes, size);
    s = api.buf.cat(s, b);
    int pad = (4 - (s.size % 4)) % 4;

    if (pad) {
      buf_t p = {};
      p.size = pad;
      s = api.buf.cat(s, p);
    }
	}
	return s;
}

buf_t serialize_string(const char *string)
{
	//buf_t b = buf_add(string, strlen(string));
	//return sel_serialize_string(b);
	return serialize_bytes(
			(ui8_t *)string, strlen(string));
}

static bool flag_is_set(int value, int flag)
{
	int flagb = (1 << flag);
	return (value & flagb) != flagb;
}
