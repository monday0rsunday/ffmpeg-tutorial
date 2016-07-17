/*
@see http://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html
*/
#include "libavformat/avformat.h"
#include <stdio.h>

int main(int argc, char* argv[]) {
	printf("Print information of video file\n");
	if(argc < 2){
		printf("Please provide path to video file\n");
	}
	av_register_all();
	AVFormatContext *fmtCtx;
	if(avformat_open_input(&fmtCtx, argv[1], NULL, NULL)){
		printf("Open file %s failed\n", argv[1]);
		return 1;
	}
	if(avformat_find_stream_info(fmtCtx, NULL)) {
		printf("Can not find stream info\n");
		return 1;
	}
	av_dump_format(fmtCtx, 0, argv[1], 0);
	avformat_close_input(&fmtCtx);
}