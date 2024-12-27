#include <stdio.h>
#include <string.h>

int ogg_to_vaw(const char *ogg, const char *wav) 
{

	OggOpusFile *file;
	int error = OPUS_OK;		
	
	// check opus file
	file = op_test_file(url.path.UTF8String, &error);	
	if (file != NULL){
		 error = op_test_open(file);
		 op_free(file);
		 if (error != OPUS_OK){
				[self.appDelegate showMessage:@"not OPUS OGG file"];
				return;
		 }
	} else {
		[self.appDelegate showMessage:@"can't open file"];
		return;
	}

	// read file to NSData
	NSMutableData *data = [NSMutableData data];
	file = op_open_file(url.path.UTF8String, &error);	
	NSAssert(file, @"op_open_file");
	int c = op_channel_count(file, -1);

	opus_int16 pcm[(160*48*c)/2];
	int size = sizeof(pcm)/sizeof(*pcm);
	while (op_read_stereo(file, pcm, size) > 0) 
	{
		[data appendBytes:pcm length:size];
	}

	FILE *fin,*fout;

	fin=fopen("ttt","r");
	fseek(fin, 0, SEEK_END);
	int len = ftell(fin);
	fseek(fin, 0, SEEK_SET);

	int readcount=0;
	short NumChannels = 2;
	short BitsPerSample = 16;
	int SamplingRate = 48000;
	short numOfSamples = 160;

	int ByteRate = NumChannels*BitsPerSample*SamplingRate/8;
	short BlockAlign = NumChannels*BitsPerSample/8;
	//int DataSize = NumChannels*numOfSamples *  BitsPerSample/8;
	int DataSize = len;
	int chunkSize = 16;
	int totalSize = 36 + DataSize;
	short audioFormat = 1;

	if((fout = fopen("1.wav", "w")) == NULL)
	{
			printf("Error opening out file ");
	}

	//totOutSample = 0;
	fwrite("RIFF", sizeof(char), 4,fout);
	fwrite(&totalSize, sizeof(int), 1, fout);
	fwrite("WAVE", sizeof(char), 4, fout);
	fwrite("fmt ", sizeof(char), 4, fout);
	fwrite(&chunkSize, sizeof(int),1,fout);
	fwrite(&audioFormat, sizeof(short), 1, fout);
	fwrite(&NumChannels, sizeof(short),1,fout);
	fwrite(&SamplingRate, sizeof(int), 1, fout);
	fwrite(&ByteRate, sizeof(int), 1, fout);
	fwrite(&BlockAlign, sizeof(short), 1, fout);
	fwrite(&BitsPerSample, sizeof(short), 1, fout);
	fwrite("data", sizeof(char), 4, fout);
	fwrite(&DataSize, sizeof(int), 1, fout);

	//while(!feof(fin))
	//{
			//fgets(buffer,sizeof(buffer),fin);
			//fputs(buffer,fout);
	//}
	char buf[BUFSIZ];
	while (fread(buf, BUFSIZ, 1, fin) > 0) {
		fwrite(buf, BUFSIZ, 1, fout);	
	}

	fclose(fin);
	fclose(fout);
}
