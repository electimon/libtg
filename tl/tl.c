#include "libtl.h"
#include "alloc.h"
#include <string.h>

char * string_from_buf(buf_t string)
{
	if (string.size < 1)
		return NULL;
	
	char *str = MALLOC(string.size + 1, return NULL);
	strncpy(str, (const char *)string.data, string.size);
	return str;
}
