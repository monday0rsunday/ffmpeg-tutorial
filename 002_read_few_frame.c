
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <stdio.h>

/**
* Write image data using simple image format ppm
* @see https://en.wikipedia.org/wiki/Netpbm_format
*/
void save_frame(AVFrame *pFrame, int width, int height, int f_idx) {
  FILE *pFile;
  char szFilename[32];
  int  y;
  
  // Open file
  sprintf(szFilename, "frame%d_.ppm", f_idx);
  pFile=fopen(szFilename, "wb");
  if(pFile==NULL)
    return;
  
  // Write header
  fprintf(pFile, "P6\n%d %d\n255\n", width, height);
  
  // Write pixel data
  for(y=0; y<height; y++)
    fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);
  
  // Close file
  fclose(pFile);
}

int main(int argc, char* argv[]) {
	printf("Read few frame and write to image\n");
	if(argc < 2) {
		printf("Missing input video file\n");
		return -1;
	}
	int ret = -1, i = 0, v_stream_idx = -1;
	char* vf_path = argv[1];
	AVFormatContext* fmt_ctx = NULL;
	AVCodecContext* codec_ctx = NULL;
	AVCodec* codec = NULL;
	AVPacket pkt;
	AVFrame* frm = NULL;

	av_register_all();
	ret = avformat_open_input(&fmt_ctx, vf_path, NULL, NULL);
	if(ret < 0){
		printf("Open video file %s failed \n", vf_path);
		goto end;
	}

	// i dont know but without this function, sws_getContext does not work
	if(avformat_find_stream_info(fmt_ctx, NULL)<0)
    	return -1;

    av_dump_format(fmt_ctx, 0, argv[1], 0);

	for(i = 0; i < fmt_ctx->nb_streams; i++) {
		if(fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			v_stream_idx = i;
			break;
		}
	}
	if(v_stream_idx == -1) {
		printf("Cannot find video stream\n");
		goto end;
	}else{
		printf("Video stream %d with resolution %dx%d\n", v_stream_idx,
			fmt_ctx->streams[i]->codecpar->width,
			fmt_ctx->streams[i]->codecpar->height);
	}

	codec_ctx = avcodec_alloc_context3(NULL);
	avcodec_parameters_to_context(codec_ctx, fmt_ctx->streams[v_stream_idx]->codecpar);

	codec = avcodec_find_decoder(codec_ctx->codec_id);
	if(codec == NULL){
		printf("Unsupported codec for video file\n");
		goto end;
	}
	ret = avcodec_open2(codec_ctx, codec, NULL);
	if(ret < 0){
		printf("Can not open codec\n");
		goto end;
	}

	frm = av_frame_alloc();

  	struct SwsContext      *sws_ctx = NULL;
  	AVFrame         *pFrameRGB = NULL;
	int             numBytes;
	uint8_t         *buffer = NULL;

  // Allocate an AVFrame structure
  pFrameRGB=av_frame_alloc();
  if(pFrameRGB==NULL)
    return -1;
  
  // Determine required buffer size and allocate buffer
  numBytes=avpicture_get_size(AV_PIX_FMT_RGB24, codec_ctx->width,
			      codec_ctx->height);
  buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

  sws_ctx =
    sws_getContext
    (
        codec_ctx->width,
        codec_ctx->height,
        codec_ctx->pix_fmt,
        codec_ctx->width,
        codec_ctx->height,
        AV_PIX_FMT_RGB24,
        SWS_BILINEAR,
        NULL,
        NULL,
        NULL
    );

	if(sws_ctx == NULL) {
		printf("Can not use sws\n");
		goto end;
	}

	avpicture_fill((AVPicture *)pFrameRGB, buffer, AV_PIX_FMT_RGB24,
		 codec_ctx->width, codec_ctx->height);



	i=0;
	int ret1 = -1, ret2 = -1, fi = -1;
	while(av_read_frame(fmt_ctx, &pkt)>=0) {
		if(pkt.stream_index == v_stream_idx) {
			ret1 = avcodec_send_packet(codec_ctx, &pkt);
			ret2 = avcodec_receive_frame(codec_ctx, frm);
			printf("ret1 %d ret2 %d\n", ret1, ret2);
			// avcodec_decode_video2(codec_ctx, frm, &fi, &pkt);
		}
		// if not check ret2, error occur [swscaler @ 0x1cb3c40] bad src image pointers
		// ret2 same as fi
		// if(fi && ++i <= 5) {
		if(ret2>= 0 && ++i <= 5) {
	        sws_scale
		        (
		            sws_ctx,
		            (uint8_t const * const *)frm->data,
		            frm->linesize,
		            0,
		            codec_ctx->height,
		            pFrameRGB->data,
		            pFrameRGB->linesize
		        );

			save_frame(pFrameRGB, codec_ctx->width, codec_ctx->height, i);
			// save_frame(frm, codec_ctx->width, codec_ctx->height, i);
		}
		av_packet_unref(&pkt);
		if(i>=5){
			break;
		}
	}

	av_frame_free(&frm);

	avcodec_close(codec_ctx);
	avcodec_free_context(&codec_ctx);
	end:
	avformat_close_input(&fmt_ctx);
	printf("Shutdown\n");
	return 0;
}