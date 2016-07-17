#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>

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
	printf("Play simple video\n");
	if(argc < 2) {
		printf("Miss input video");
		return -1;
	}
	int ret = -1, i = -1, v_stream_idx = -1;
	char* vf_path = argv[1];
	// fuck, fmt_ctx must be inited by NULL
	AVFormatContext* fmt_ctx = NULL;
	AVCodecContext* codec_ctx = NULL;
	AVCodec* codec;
	AVFrame * frame;
	AVPacket packet;

	av_register_all();
	ret = avformat_open_input(&fmt_ctx, vf_path, NULL, NULL);
	if(ret < 0){
		printf("Open video file %s failed \n", vf_path);
		goto end;
	}
	if(avformat_find_stream_info(fmt_ctx, NULL)<0)
    	goto end;
    av_dump_format(fmt_ctx, 0, vf_path, 0);
    for(i = 0; i< fmt_ctx->nb_streams; i++) {
    	if(fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
    		v_stream_idx = i;
    		break;
    	}
    }
    if(v_stream_idx == -1) {
		printf("Cannot find video stream\n");
		goto end;
	}

	codec_ctx = avcodec_alloc_context3(NULL);
	avcodec_parameters_to_context(codec_ctx, fmt_ctx->streams[v_stream_idx]->codecpar);
	codec = avcodec_find_decoder(codec_ctx->codec_id);
	if(codec == NULL){
		printf("Unsupported codec for video file\n");
		goto end;
	}
	if(avcodec_open2(codec_ctx, codec, NULL) < 0){
		printf("Can not open codec\n");
		goto end;
	}

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
    	printf("Could not init SDL due to %s", SDL_GetError());
    	goto end;
    }
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Event event;
    SDL_Rect r;
    window = SDL_CreateWindow("SDL_CreateTexture", SDL_WINDOWPOS_UNDEFINED,
    	SDL_WINDOWPOS_UNDEFINED, codec_ctx->width, codec_ctx->height,
    	SDL_WINDOW_RESIZABLE);
    r.x = 0;
    r.y = 0;
    r.w = codec_ctx->width;
    r.h = codec_ctx->height;

    renderer = SDL_CreateRenderer(window, -1, 0);
    // texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
    // 	codec_ctx->width, codec_ctx->height);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING,
    	codec_ctx->width, codec_ctx->height);

    struct SwsContext      *sws_ctx = NULL;
    sws_ctx = sws_getContext(codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
    	codec_ctx->width, codec_ctx->height, AV_PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);

    frame = av_frame_alloc();

    int ret1, ret2;
    AVFrame* pict;
    pict = av_frame_alloc();


	int             numBytes;
	uint8_t         *buffer = NULL;
  	numBytes=avpicture_get_size(AV_PIX_FMT_YUV420P, codec_ctx->width,
			      codec_ctx->height);
  	buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
    // required, or bad dst image pointers
	avpicture_fill((AVPicture *)pict, buffer, AV_PIX_FMT_YUV420P,
		 codec_ctx->width, codec_ctx->height);
    i = 0;
	while (1) {
        SDL_PollEvent(&event);
        if(event.type == SDL_QUIT)
                break;
        ret = av_read_frame(fmt_ctx, &packet);
        if(ret <0){
        	continue;
        }
        if(packet.stream_index == v_stream_idx) {
			ret1 = avcodec_send_packet(codec_ctx, &packet);
			ret2 = avcodec_receive_frame(codec_ctx, frame);
			if(ret2 < 0 ){
				continue;
	    	}
	    	sws_scale(sws_ctx, (uint8_t const * const *)frame->data,
			      frame->linesize, 0, codec_ctx->height,
			      pict->data, pict->linesize);
	   //  	if(++i <=5 ){
				// save_frame(pict, codec_ctx->width, codec_ctx->height, i);
	   //  	}
	        SDL_UpdateYUVTexture(texture, &r, pict->data[0], pict->linesize[0],
	        	pict->data[1], pict->linesize[1],
	        	pict->data[2], pict->linesize[2]);
	        // SDL_UpdateTexture(texture, &r, pict->data[0], pict->linesize[0]);

	        // r.x=rand()%500;
	        // r.y=rand()%500;

	        // SDL_SetRenderTarget(renderer, texture);
	        // SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
	        SDL_RenderClear(renderer);
	        // SDL_RenderDrawRect(renderer,&r);
	        // SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0x00);
	        // SDL_RenderFillRect(renderer, &r);
	        // SDL_SetRenderTarget(renderer, NULL);
	        SDL_RenderCopy(renderer, texture, NULL, NULL);
	        // SDL_RenderCopy(renderer, texture, &r, &r);
	        SDL_RenderPresent(renderer);
	        // SDL_Delay(50);
        }
        av_packet_unref(&packet);
    }

    SDL_DestroyRenderer(renderer);
    SDL_Quit();
	av_frame_free(&frame);
	avcodec_close(codec_ctx);
	avcodec_free_context(&codec_ctx);
    end:
	avformat_close_input(&fmt_ctx);
	printf("Shutdown\n");
	return 0;
}