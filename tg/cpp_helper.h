#ifndef CPP_HELPER_H_
#define CPP_HELPER_H_

#include <stddef.h>
typedef struct std_string_ {
	size_t _M_length;
	size_t _M_capacity;
	int    _M_refcount;
	char   *str;
} std_string;

#endif /* ifndef CPP_HELPER_H_ */
