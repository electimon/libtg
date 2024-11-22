#include "buf.h"
#include "alloc.h"
#include <zlib.h>
int gunzip_buf(buf_t *dst, buf_t src){
	// debug
	/*printf("%s\n", __func__);*/
	/*buf_dump(src);*/
	
	// allocte data
	buf_init(dst);
	buf_realloc(dst, src.size * 4);
	
	z_stream s;
	s.zalloc    = Z_NULL;
	s.zfree     = Z_NULL;
	s.opaque    = Z_NULL;
	s.avail_in  = src.size;
	s.next_in   = src.data;
	s.avail_out = dst->asize;
	s.next_out  = dst->data;
	if (inflateInit2(&s, 16 + MAX_WBITS) != Z_OK){
		printf("can't init inflate\n");
		return 1;
	} 

	int ret = inflate(&s, Z_FINISH);  

	if (ret != Z_OK && ret != Z_STREAM_END){
		switch (ret) {
			case Z_BUF_ERROR:
				printf("uncompress error:"
						" no progress is possible; either avail_in: "
						"or avail_out was zero\n");
				break;
			case Z_MEM_ERROR:
				printf("uncompress error: Insufficient memory\n");
				break;
			case Z_STREAM_ERROR:
				printf("uncompress error: The state (as "
						"represented in stream) is inconsistent, "
						"or stream was NULL\n");
				break;
			case Z_NEED_DICT:
				printf("uncompress error: A preset dictionary"
						" is required. The adler field shall be set to"
						" the Adler-32 checksum of the dictionary"
						" chosen by the compressor\n");
				break;
			case Z_DATA_ERROR:
				printf("uncompress error: data is corrupted\n");
				break;
			
			default:
				printf("uncompress error: %d\n", ret);
				break;
		}
		return 1;	
	}

	/*printf("total_out: %ld\n", s.total_out);*/
	inflateEnd(&s);

	dst->size = s.total_out;

	/*printf("done\n");*/
	return 0;
}
