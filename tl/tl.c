#include "libtl.h"
#include "alloc.h"
#include <string.h>

const char * string_from_buf(buf_t string)
{
	if (string.size < 1)
		return NULL;
	
	return (const char *)string.data;
}
