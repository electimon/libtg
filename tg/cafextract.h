#ifndef CAF_EXTRACT_H
#define CAF_EXTRACT_H

#include <stdint.h>

struct CAFEAudioFormat {
	double    mSampleRate;
	uint32_t  mFormatID;
	uint32_t  mFormatFlags;
	uint32_t  mBytesPerPacket;
	uint32_t  mFramesPerPacket;
	uint32_t  mChannelsPerFrame;
	uint32_t  mBitsPerChannel;
};

enum {
	keAudioFormatLinearPCM      = 'lpcm',
	keAudioFormatAppleIMA4      = 'ima4',
	keAudioFormatMPEG4AAC       = 'aac ',
	keAudioFormatMACE3          = 'MAC3',
	keAudioFormatMACE6          = 'MAC6',
	keAudioFormatULaw           = 'ulaw',
	keAudioFormatALaw           = 'alaw',
	keAudioFormatMPEGLayer1     = '.mp1',
	keAudioFormatMPEGLayer2     = '.mp2',
	keAudioFormatMPEGLayer3     = '.mp3',
	keAudioFormatAppleLossless  = 'alac'
};

enum {
 CE_OK,
 CE_READ_ERROR,
 CE_WRITE_ERROR,
 CE_FILE_CORRUPTED,
};

int caf_extract(
		const char *caf, 
		struct CAFEAudioFormat *format,
		const char *extracted);

#endif /* ifndef CAF_EXTRACT_H */
