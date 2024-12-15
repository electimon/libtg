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
	void *data;
	int (*callback)(void *data, const tg_file_t *file);
	void *progressp;
	void (*progress)(void *progressp, int down, int total);
};

static int _tg_get_file_cb(void *userdata, const tl_t *tl){
	struct tg_get_file_t *s = userdata;
	if (tl && tl->_id == id_upload_file){
			tg_file_t file;
			memset(&file, 0, sizeof(file));
			tg_file_from_tl(&file, tl);
			if (s->callback)
				s->callback(s->data, &file);
	} else {
	// throw error
		if (tl)
			printf("ID: %.8x\n", tl->_id);
		char *err = tg_strerr(tl); 
		ON_ERR(s->tg, "%s", err);
		free(err);
	}

	return 0;
}

static buf_t _tg_get_file_chunk(void *chunkp, uint32_t received, uint32_t total)
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

void tg_get_file(
		tg_t *tg, 
		InputFileLocation *location,
	  void *data,
	  int (*callback)(void *data, const tg_file_t *file),
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
				return);

	s->tg = tg;
	s->limit  = 1048576;
	s->location = *location;
	s->data = data;
	s->callback = callback;
	s->progressp = progressp;
	s->progress = progress;

	tl_t *tl = NULL;
	
	// download parts of file
	buf_t getFile = tl_upload_getFile(
			NULL, 
			NULL, 
			location, 
			0, 
			s->limit);
		
	// net send
	tg_queue_manager_send_query(
			tg, 
			getFile,
			s,
			_tg_get_file_cb,
			s, 
			_tg_get_file_chunk);
}

struct photo_file_t {
	tg_t *tg;
	bool big_photo;
	uint64_t peer_id; 
	uint64_t photo_id; 
	uint64_t photo_access_hash;
	char photo_size[2];
	void *userdata;
	int (*callback)(void *userdata, char *photo);
};

static int get_photo_callback(void *d, const tg_file_t *p)
{
	if (!p)
		return 0;

	struct photo_file_t *s = d;
	// save photo to base
	if ((!s->big_photo) || 
			 strcmp(s->photo_size, "s") == 0)
	{
		 peer_photo_to_database(
				 s->tg, s->peer_id,
				 s->photo_id, p->bytes_);
	}
	if (s->callback)
		s->callback(s->userdata, p->bytes_);

	free(s);
	return 0;
}

void tg_get_photo_file(tg_t *tg, 
		uint64_t photo_id, uint64_t photo_access_hash, 
		const char *photo_file_reference,
		const char *photo_size,
		void *userdata,
		int (*callback)(void *userdata, char *photo))
{
	char *photo = NULL;
	if (strcmp(photo_size, "s") == 0){
		photo = photo_file_from_database(tg, photo_id);
		if (photo){
			if (callback)
				callback(userdata, photo);
			return;
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

	struct photo_file_t *s = 
		NEW(struct photo_file_t, 
				ON_ERR(tg, "%s: can't allocate memory", __func__); return);

	s->tg = tg;
	s->photo_id = photo_id;
	s->userdata = userdata;
	s->callback = callback;
	strncpy(s->photo_size, photo_size, 1);

	 tg_get_file(
			tg, 
			&location, 
			&photo, 
			get_photo_callback, 
			NULL, 
			NULL);
}

void tg_get_peer_photo_file(tg_t *tg, 
		tg_peer_t *peer, 
		bool big_photo,
		uint64_t photo_id, 
		void *userdata,
		int (*callback)(void *userdata, char *photo))
{
	fprintf(stderr, "%s\n", __func__);
	char *photo = NULL;
	if (!big_photo){
		photo = peer_photo_file_from_database(
				tg, peer->id, photo_id);
		if (photo){
			if (callback)
				callback(userdata, photo);
			return;
		}
	}
	buf_t peer_ = tg_inputPeer(*peer);
	InputFileLocation location = 
		tl_inputPeerPhotoFileLocation(
				true, 
				&peer_, 
				photo_id);
	buf_free(peer_);


	struct photo_file_t *s = 
		NEW(struct photo_file_t, 
				ON_ERR(tg, "%s: can't allocate memory", __func__); return);

	s->tg = tg;
	s->big_photo = big_photo;
	s->peer_id = peer->id;
	s->photo_id = photo_id;
	s->userdata = userdata;
	s->callback = callback;

	tg_get_file(
			tg, 
			&location, 
			s, 
			get_photo_callback, 
			NULL, 
			NULL);
}
