#include "files.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../transport/transport.h"
#include "../transport/net.h"
#include "../tl/alloc.h"
#include "tg.h"

static void tg_file_from_tl(tg_file_t *f, const tl_t *tl)
{
	fprintf(stderr, "%s\n", __func__);
	if (!tl || tl->_id != id_upload_file)
		return;

	tl_upload_file_t *uf = (tl_upload_file_t *)tl;
	
	#define TG_FILE_TYP(t, arg, ...) f->arg = uf->arg->_id;
	#define TG_FILE_ARG(t, arg, ...) f->arg = uf->arg;
	#define TG_FILE_BUF(t, arg, ...) f->arg = buf_add_buf(uf->arg);
	TG_FILE_ARGS
	#undef TG_FILE_TYP
	#undef TG_FILE_ARG
	#undef TG_FILE_BUF
}

int tg_get_file_with_progress(
		tg_t *tg, 
		InputFileLocation *location,
		int size,
		void *userdata,
		int (*callback)(
			void *userdata, const tg_file_t *file),
		void *progressp,
			int (*progress)(void *progressp, int size, int total))
{
	printf("%s start\n", __func__);
	/* If precise flag is not specified, then

		• The parameter offset must be divisible by 4 KB.
		• The parameter limit must be divisible by 4 KB.
		• 1048576 (1 MB) must be divisible by limit.

		 If precise is specified, then

		• The parameter offset must be divisible by 1 KB.
		• The parameter limit must be divisible by 1 KB.
		• limit must not exceed 1048576 (1 MB).
	 */

	/* In any case the requested part should be within 
	 * one 1 MB chunk from the beginning of the file, i. e.
   • offset / (1024 * 1024) == (offset + limit - 1) / (1024 * 1024).
	 */

	/*int i, limit = 1024*4, offset = 0; // for testing */
	int i, limit = 1048576, offset = 0;
		
	tl_t *tl = NULL;

	for (i = 0; size>0?offset<size:1; ++i) 
	{
		printf("%s: download total: %d with offset: %d (%d%%)\n",
				__func__, size, offset, offset/size);
		// download parts of file
		buf_t getFile = tl_upload_getFile(
				NULL, 
				NULL, 
				location, 
				offset, 
				limit);
			
		// net send
		tl = tg_run_api_with_progress(
				tg, &getFile, progressp, progress);
		buf_free(getFile);

		if (tl == NULL)
			return offset;

		if (tl->_id != id_upload_file){
			ON_ERR(tg, "%s: exceed upload_file but got: %s", 
					__func__, TL_NAME_FROM_ID(tl->_id));
			return offset;
		}
		
		tg_file_t file;
		memset(&file, 0, sizeof(file));
		tg_file_from_tl(&file, tl);
		
		// add offset
		offset += file.bytes_.size;

		printf("FILE TYPE: %s\n", TL_NAME_FROM_ID(file.type_));
		if (callback)
			if (callback(userdata, &file))
				break;

		tg_file_free(&file);
		
		// free tl
		tl_free(tl);
	}
	
	/*return file;*/
	return offset;
}

int tg_get_file(
		tg_t *tg, 
		InputFileLocation *location,
		int size,
		void *userdata,
		int (*callback)(
			void *userdata, const tg_file_t *file))
{
	return tg_get_file_with_progress(
			tg, 
			location, 
			size, 
			userdata, 
			callback, 
			NULL, 
			NULL);
}

static int _photo_file_cb(void *userdata, const tg_file_t *file)
{
	char **photo = userdata;
	*photo = buf_to_base64(file->bytes_); 
	return 1;
}

char * tg_get_photo_file(tg_t *tg, 
		uint64_t photo_id, uint64_t photo_access_hash, 
		const char *photo_file_reference,
		const char *photo_size)
{
	fprintf(stderr, "%s\n", __func__);
	
	char *photo = NULL;
	
	//if (strcmp(photo_size, "s") == 0){
		//photo = photo_file_from_database(tg, photo_id);
		//if (photo){
			//return photo;
		//}
	//}

	buf_t fr = buf_from_base64(photo_file_reference);
	InputFileLocation location = 
					tl_inputPhotoFileLocation(
							photo_id, 
							photo_access_hash, 
							&fr, 
							photo_size);
	buf_free(fr);

	tg_get_file(
			tg, 
			&location, 
			0,
			&photo, 
			_photo_file_cb);
	buf_free(location);
	
	//if (photo == NULL)
		//return NULL;

	//if (strcmp(photo_size, "s") == 0)
		//photo_to_database(tg, photo_id, photo);

	return photo;
}

char * tg_get_peer_photo_file(tg_t *tg, 
		tg_peer_t *peer, 
		bool big_photo,
		uint64_t photo_id) 
{
	fprintf(stderr, "%s\n", __func__);
	
	char *photo = NULL;
	
	//if (!big_photo){
		//photo = peer_photo_file_from_database(
				//tg, peer->id, photo_id);
		//if (photo){
			//return photo;
		//}
	//}
	buf_t peer_ = tg_inputPeer(*peer);
	InputFileLocation location = 
		tl_inputPeerPhotoFileLocation(
				true, 
				&peer_, 
				photo_id);
	buf_free(peer_);

	tg_get_file(
			tg, 
			&location, 
			0,
			&photo, 
			_photo_file_cb);
	buf_free(location);
	
	//if (photo == NULL)
		//return NULL;
	
	//if (!big_photo)
		//peer_photo_to_database(tg, peer->id, 
				//photo_id, photo);

	return photo;
}

void tg_get_document(tg_t *tg, 
		uint64_t id, 
		uint64_t size, 
		uint64_t access_hash, 
		const char * file_reference, 
		void *userdata,
		int (*callback)(
			void *userdata, const tg_file_t *file),	
		void *progressp,
			int (*progress)(void *progressp, int size, int total))
{
	buf_t fr = buf_from_base64(file_reference);
	InputFileLocation location =
		tl_inputDocumentFileLocation(
				id, 
				access_hash, 
				&fr, 
				"");
	buf_free(fr);

	tg_get_file_with_progress(
			tg, 
			&location, 
			size,
			userdata, 
			callback,
			progressp,
			progress);

	buf_free(location);
}	

char * tg_get_document_get_thumb(tg_t *tg, 
		uint64_t id, 
		uint64_t size, 
		uint64_t access_hash, 
		const char * file_reference, 
		const char * thumb_size)
{	
	fprintf(stderr, "%s\n", __func__);
	
	char *photo = NULL;
	
	buf_t fr = buf_from_base64(file_reference);
	InputFileLocation location =
		tl_inputDocumentFileLocation(
				id, 
				access_hash, 
				&fr, 
				thumb_size);
	buf_free(fr);

	tg_get_file(
			tg, 
			&location, 
			size,
			&photo, 
			_photo_file_cb);

	buf_free(location);

	return photo;
}

void tg_file_free(tg_file_t *f){
	#define TG_FILE_TYP(...)
	#define TG_FILE_ARG(...)
	#define TG_FILE_BUF(t, n, ...) buf_free(f->n); 
	TG_FILE_ARGS
	#undef TG_FILE_TYP
	#undef TG_FILE_ARG
	#undef TG_FILE_BUF
}
