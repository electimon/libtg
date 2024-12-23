#ifndef TG_FILES_H
#define TG_FILES_H

#include "tg.h"
#include "peer.h"

#define TG_FILE_ARGS\
	TG_FILE_TYP(uint32_t, type_,  "INT",  "type") \
	TG_FILE_ARG(uint32_t, mtime_, "INT",  "mtime") \
	TG_FILE_STR(char*,    bytes_, "TEXT", "bytes") \


typedef enum {
	TG_STORAGE_FILETYPE_NULL     = 0,
	TG_STORAGE_FILETYPE_UNKNOWN  = id_storage_fileUnknown, //Unknown type.
	TG_STORAGE_FILETYPE_PARTIRAL = id_storage_filePartial, // Part of a bigger file.
	TG_STORAGE_FILETYPE_JPEG     = id_storage_fileJpeg,    // JPEG image. MIME type: image/jpeg.
	TG_STORAGE_FILETYPE_GIF      = id_storage_fileGif,     // GIF image. MIME type: image/gif.
	TG_STORAGE_FILETYPE_PNG      = id_storage_filePng,     // PNG image. MIME type: image/png.
	TG_STORAGE_FILETYPE_PDF      = id_storage_filePdf,     // PDF document image. MIME type: application/pdf.
	TG_STORAGE_FILETYPE_MP3      = id_storage_fileMp3,     //Mp3 audio. MIME type: audio/mpeg.
	TG_STORAGE_FILETYPE_MOV      = id_storage_fileMov,     // Quicktime video. MIME type: video/quicktime.
	TG_STORAGE_FILETYPE_MP4      = id_storage_fileMp4,     // MPEG-4 video. MIME type: video/mp4.
	TG_STORAGE_FILETYPE_WEBP     = id_storage_fileWebp,    // WEBP image. MIME type: image/webp.
} TG_STORAGE_FILETYPE;

typedef struct tg_file_ {
	#define TG_FILE_TYP(t, arg, ...) t arg;
	#define TG_FILE_ARG(t, arg, ...) t arg;
	#define TG_FILE_STR(t, arg, ...) t arg; 
	TG_FILE_ARGS
	#undef TG_FILE_TYP
	#undef TG_FILE_ARG
	#undef TG_FILE_STR
} tg_file_t;

tg_file_t * tg_get_file(
		tg_t *tg, 
		InputFileLocation *location,
		void *progressp,
		void (*progress)(void *progressp, int down, int total));	

char * tg_get_photo_file(tg_t *tg, 
		uint64_t photo_id, uint64_t photo_access_hash, 
		const char *photo_file_reference,
		const char *photo_size);

char * tg_get_peer_photo_file(tg_t *tg, 
		tg_peer_t *peer, 
		bool big_photo,
		uint64_t photo_id); 

#endif /* ifndef TG_FILES_H */
