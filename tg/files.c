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
	#define TG_FILE_STR(t, arg, ...) f->arg = buf_to_base64(uf->arg); 
	TG_FILE_ARGS
	#undef TG_FILE_TYP
	#undef TG_FILE_ARG
	#undef TG_FILE_STR
}

struct tg_get_file_t{
	tg_t *tg;
	uint32_t limit;
	InputFileLocation location;
	void *progressp;
	void (*progress)(void *progressp, int down, int total);
};

static buf_t _tg_get_file_chunk(
		void *chunkp, uint32_t received, uint32_t total)
{
	struct tg_get_file_t *s = chunkp;
	if (s->progress)
		s->progress(s->progressp, received, total);
	
	buf_t getFile = tl_upload_getFile(
					NULL, 
					NULL, 
					&s->location, 
					received, 
					s->limit);
	return getFile;
}

tg_file_t * tg_get_file(
		tg_t *tg, 
		InputFileLocation *location,
		void *progressp,
		void (*progress)(void *progressp, int down, int total))	
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

	struct tg_get_file_t *s = 
		NEW(struct tg_get_file_t, 
				ON_ERR(tg, "%s: can't allocate memory", __func__);
				return NULL);

	s->tg = tg;
	s->limit  = 1048576;
	s->location = buf_add_buf(*location);
	s->progressp = progressp;
	s->progress = progress;

	// download parts of file
	buf_t getFile = tl_upload_getFile(
			NULL, 
			NULL, 
			location, 
			0, 
			s->limit);
		
	// net send
	tl_t *tl = tg_send_api(
			tg, &getFile, 
			s, _tg_get_file_chunk);
	buf_free(getFile);

	if (tl == NULL)
		return NULL;
	
	if (tl->_id != id_upload_file){
		ON_ERR(tg, "%s: exceed upload_file but got: %s", 
				__func__, TL_NAME_FROM_ID(tl->_id));
		return NULL;
	}

	tg_file_t *file = NEW(tg_file_t, 
			ON_ERR(tg, "%s: can't allocate memory", __func__);
			return NULL);
	tg_file_from_tl(file, tl);
	free(s);
	return file;
}

char * tg_get_photo_file(tg_t *tg, 
		uint64_t photo_id, uint64_t photo_access_hash, 
		const char *photo_file_reference,
		const char *photo_size)
{
	if (strcmp(photo_size, "s") == 0){
		char *photo = photo_file_from_database(tg, photo_id);
		if (photo){
			return photo;
		}
	}

	buf_t fr = buf_from_base64(photo_file_reference);
	InputFileLocation location = 
					tl_inputPhotoFileLocation(
							photo_id, 
							photo_access_hash, 
							&fr, 
							photo_size);
	buf_free(fr);

	tg_file_t *file = tg_get_file(
			tg, 
			&location, 
			NULL, 
			NULL);
	buf_free(location);
	
	if (file == NULL)
		return NULL;

	if (strcmp(photo_size, "s") == 0)
		photo_to_database(tg, photo_id, file->bytes_);

	char *photo = file->bytes_;
	free(file);
	
	return photo;
}

char * tg_get_peer_photo_file(tg_t *tg, 
		tg_peer_t *peer, 
		bool big_photo,
		uint64_t photo_id) 
{
	fprintf(stderr, "%s\n", __func__);
	if (!big_photo){
		char *photo = peer_photo_file_from_database(
				tg, peer->id, photo_id);
		if (photo){
			return photo;
		}
	}
	buf_t peer_ = tg_inputPeer(*peer);
	InputFileLocation location = 
		tl_inputPeerPhotoFileLocation(
				true, 
				&peer_, 
				photo_id);
	buf_free(peer_);

	tg_file_t *file = tg_get_file(
			tg, 
			&location, 
			NULL, 
			NULL);
	buf_free(location);
	
	if (file == NULL)
		return NULL;
	
	if (!big_photo)
		peer_photo_to_database(tg, peer->id, 
				photo_id, file->bytes_);

	char *photo = file->bytes_;
	free(file);
	
	return photo;
}
