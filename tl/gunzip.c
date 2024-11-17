#include "../mtx/include/buf.h"
#include "alloc.h"
#include <zlib.h>
int gunzip_buf(buf_t *dst, buf_t src){
	// debug
	printf("%s\n", __func__);
	buf_dump(src);
	
	// allocte data
	unsigned char *data = 
		MALLOC(src.size * 2, return 1);
	
	z_stream s;
	s.zalloc    = Z_NULL;
	s.zfree     = Z_NULL;
	s.opaque    = Z_NULL;
	s.avail_in  = src.size;
	s.next_in   = src.data;
	s.avail_out = src.size * 2;
	s.next_out  = data;
	if (inflateInit2(&s, 16 + MAX_WBITS) != Z_OK){
		printf("can't init inflate\n");
		return 1;
	} 

	int ret = inflate(&s, Z_FINISH);  

	if (ret != Z_OK && ret != Z_STREAM_END){
		printf("uncompress error: %d\n", ret);
		return 1;	
	}

	printf("total_out: %ld\n", s.total_out);
	inflateEnd(&s);

	*dst = buf_add(data, s.total_out);

	printf("done\n");
	return 0;
}
