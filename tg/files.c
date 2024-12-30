#include "files.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <openssl/md5.h>
#include "../transport/transport.h"
#include "../transport/net.h"
#include "../tl/alloc.h"
#include "peer.h"
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
			ON_ERR(tg, "%s: expected upload_file but got: %s", 
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

char * tg_get_document_thumb(tg_t *tg, 
		uint64_t id, 
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
			0,
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

struct tg_document_send_with_progress_progress_t {
	int total;
	int current;
	void *progressp;
	int (*progress)(void *, int, int);
};

static int tg_document_send_with_progress_progress(
		void *p, int size, int total)
{
	assert(p);
	struct tg_document_send_with_progress_progress_t *t = 
		(struct tg_document_send_with_progress_progress_t *)p;
	t->current += size;
	return t->progress(t->progressp, t->current, t->total);
}

int tg_document_send(
		tg_t *tg, tg_peer_t *peer, 
		const char *filename,
		const char *filepath,
		bool isAnimation,
		const char *mime_type,
		const char *message,
		void *progressp, int (*progress)(void *, int, int))
{
	int i;
	ON_LOG(tg, "%s...", __func__);
	FILE *fp = fopen(filepath, "r");
	if (fp == NULL){
		ON_ERR(tg, "%s: can't open file: %s", __func__, filepath);
		return 1;
	}
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// Before transmitting the contents of the file itself, 
	// the file has to be assigned a unique 64-bit 
	// client identifier: file_id.
	buf_t file_id_ = buf_rand(8);
	uint64_t file_id = buf_get_ui64(file_id_);
	buf_free(file_id_);

	// The file's binary content is then split into parts. 
	// All parts must have the same size (part_size) and 
	// the following conditions must be met:
	// • part_size % 1024 = 0 (divisible by 1KB)
  // • 524288 % part_size = 0 (512KB must be evenly divisible 
	// by part_size)
	//
	// The last part does not have to satisfy these conditions, 
	// provided its size is less than part_size.
	// The following appConfig fields specify the 
	// maximum number of uploadable file parts:
	// • upload_max_fileparts_default » - Maximum number of 
	//	 file parts uploadable by non-Premium users.
	// • upload_max_fileparts_premium » - Maximum number of 
	//   file parts uploadable by Premium users.
	//
	// Note that the limit of uploadable file parts does 
	// not account for the part_size: thus the total file 
	// size limit can only be reached with the biggest possible
	//
	// part_size of 512KB, which is actually the 
	// recommended part_size to avoid excessive protocol overhead.
	int part_size =  524288;
	
	// Each part should have a sequence number, file_part, 
	// with a value ranging from 0 to the value of the 
	// appropriate config parameter minus 1.
	uint32_t file_part = 0;

	// After the file has been partitioned you need to 
	// choose a method for saving it on the server. 
	// Use upload.saveBigFilePart in case the full size of 
	// the file is more than 10 MB and upload.saveFilePart 
	// for smaller files.
	buf_t buf = buf_new();
	ON_LOG(tg, "%s: prepare file: %s with size: %d", 
			__func__, filename, size);
	if (size > 10485760){
		// save big file
		/* TODO:  <30-12-24, yourname> */
	} else {
		// save file part
		buf_t bytes = buf_new();
		buf_realloc(&bytes, part_size);
		
		struct tg_document_send_with_progress_progress_t t =
			{size, 0, progressp, progress};
		
		int len, current = 0, retry = 0;
		for (len = fread(bytes.data, 1, part_size, fp);
				 len > 0;
				 len = fread(bytes.data, 1, part_size, fp))
		{
			bytes.size = part_size;
			buf = buf_cat(buf, bytes);
			
tg_document_send_with_progress_saveFilePart:;
			ON_LOG(tg, "%s: upload %d part of file: %s", __func__, file_part, filepath);
			if (retry > 9)
			{
				ON_ERR(tg, "%s: can't upload file: %s (retries > 10)", __func__, filepath);
			}

			buf_t saveFilePart = tl_upload_saveFilePart(
					file_id, 
					file_part, 
					&bytes);

			tl_t *tl = tg_run_api_with_progress(
					tg, 
					&saveFilePart, 
					&t, 
					tg_document_send_with_progress_progress);
			buf_free(saveFilePart);

			if (tl == NULL || tl->_id != id_boolTrue){
				ON_ERR(tg, "%s: expected tl_true but got: %s", 
						__func__, TL_NAME_FROM_ID(tl->_id));
				// retry
				retry++;
				goto tg_document_send_with_progress_saveFilePart;
			}

			file_part++;
		}	
		buf_free(bytes);
	}

	// While the parts are being uploaded, an MD5 hash of 
	// the file contents can also be computed to be used 
	// later as the md5_checksum parameter in the inputFile 
	// constructor (since it is checked only by the server, 
	// for encrypted secret chat files it must be generated 
	// from the encrypted file). 
	unsigned char md5[MD5_DIGEST_LENGTH];
	assert(MD5(buf.data, buf.size, md5));
	char md5_checksum[BUFSIZ] = {0};
	for (i = 0; i < MD5_DIGEST_LENGTH; ++i) {
		sprintf(md5_checksum, "%s%02x", md5_checksum, md5[i]); 
	}
	ON_LOG(tg, "%s: file MD5 hash: %s", __func__, md5_checksum);

	// After the entire file is successfully saved, the final
	// method may be called and passed the generated 
	// inputFile object. In case the upload.saveBigFilePart 
	// method is used, the inputFileBig constructor must 
	// be passed, in other cases use inputFile.
	if (size > 10485760){
		/* TODO:  <30-12-24, yourname> */
	} else {
		
		buf_t inputFile = tl_inputFile(
				file_id, 
				file_part, 
				filename?filename:"", 
				md5_checksum);
	
		InputMedia media = tl_inputMediaUploadedDocument(
				false, 
				false, 
				false, 
				&inputFile, 
				NULL, 
				mime_type?mime_type:"", 
				NULL, 
				0, 
				NULL, 
				0, 
				NULL);
		buf_free(inputFile);

		buf_t peer_ = tg_inputPeer(*peer);
		buf_t random_id = buf_rand(8);

		buf_t sendMedia = tl_messages_sendMedia(
				false, 
				false, 
				false, 
				false, 
				false, 
				false, 
				&peer_, 
				NULL, 
				&media, 
				message, 
				buf_get_ui64(random_id), 
				NULL, 
				NULL, 
				0, 
				NULL, 
				NULL, 
				NULL, 
				NULL);
		buf_free(media);
		buf_free(peer_);
		buf_free(random_id);

		tl_t *tl = tg_run_api(tg, &sendMedia);
		buf_free(sendMedia);

		if (tl == NULL){
			return 1;
		}

		/* TODO: handle tl_updates <30-12-24, yourname> */
	}

	return 0;
}	
