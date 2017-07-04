/****************************************************************************

Демо-программа №1

Демонстрирует возможности модификации изображения при перекодирования

****************************************************************************/

#include <ffcairo/config.h>
#include <ffcairo/ffcimage.h>
#include <ffcairo/ffcstream.h>

void ModifyFrame(FFCImage *pic, int iFrame)
{
	// создаем контекст рисования Cairo
	cairo_t *cr = cairo_create(pic->surface);
	
	char sFrameId[48];
	sprintf(sFrameId, "Frame: %03d", iFrame);
	
	double hbox = pic->height * 0.2;
	double shift = pic->height * 0.1;
	
	cairo_set_source_rgba (cr, 0x5a /255.0, 0xe8/255.0, 0xf9/255.0, 96/255.0);
	cairo_rectangle (cr, shift, pic->height - hbox - shift, pic->width - 2*shift, hbox);
	cairo_fill (cr);
	
	cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(cr, hbox * 0.8);
	cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
	
	cairo_move_to (cr, shift*1.5, pic->height - shift*1.5);
	cairo_show_text (cr, sFrameId);
	
	// Освобождаем контекст рисования Cairo
	cairo_destroy(cr);
}

int main(int argc, char *argv[])
{
	// INIT
	av_register_all();
	avformat_network_init();
	
	const char *fname = argv[1];
	
	FFCInput *vin = new FFCInput();
	if ( ! vin->open(fname) ) return -1;
	
	printf("video stream #%d\n", vin->videoStream);
	
	// Allocate video frame
	AVFrame *pFrame = av_frame_alloc();
	
	AVCodecContext *pCodecCtx = vin->videoCodecCtx;
	if ( pCodecCtx == NULL ) return -1;
	
	printf("video size: %dx%d\n", pCodecCtx->width, pCodecCtx->height);
	
	FFCImage *pic = new FFCImage(pCodecCtx->width, pCodecCtx->height);
	
	struct SwsContext *sws_ctx = NULL;
	int frameFinished;
	AVPacket packet;
	
	// initialize SWS context for software scaling
	sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, PIX_FMT_BGRA, SWS_BILINEAR,
		NULL, NULL, NULL);
	
	int i=0;
	while( av_read_frame(vin->avFormatCtx, &packet) >= 0 )
	{
		// Is this a packet from the video stream?
		if( packet.stream_index == vin->videoStream )
		{
			// Decode video frame
			avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
			
			// Did we get a video frame?
			if(frameFinished) {
			// Convert the image from its native format to RGB
				sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data,
				pFrame->linesize, 0, pCodecCtx->height,
				pic->avFrame->data, pic->avFrame->linesize);
			
				// Save the frame to disk
				if(++i<10)
				{
					ModifyFrame(pic, i);
					
					// сохраняем фрейм в файл
					char szFilename[48];
					sprintf(szFilename, "out/frame%03d.png", i);
					pic->savePNG(szFilename);
					
					if ( i % 50 == 0 ) printf("#%i\n", i);
				}
				else
				{
					av_free_packet(&packet);
					break;
				}
			}
		}
		
		// Free the packet that was allocated by av_read_frame
		av_free_packet(&packet);
	}
	
	return 0;
}
