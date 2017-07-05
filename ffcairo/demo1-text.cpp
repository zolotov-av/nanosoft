/****************************************************************************

Демо-программа №1

Демонстрирует возможности модификации изображения при перекодирования

****************************************************************************/

#include <ffcairo/config.h>
#include <ffcairo/ffcimage.h>
#include <ffcairo/ffcdemuxer.h>

class MyInputStream: public FFCDecodedInput
{
public:
	int frameNo;
	FFCImage *pic;
	
	struct SwsContext *sws_ctx;
	
	/**
	 * Обработчик фрейма
	 */
	virtual void handleFrame();
	
	void ModifyFrame(FFCImage *pic, int iFrame);
};

void MyInputStream::handleFrame()
{
	frameNo ++;
	
	// Save the frame to disk
	if( frameNo < 10 )
	{
		printf("handleFrame #%d\n", frameNo);
		
		sws_scale(sws_ctx, (uint8_t const * const *)avFrame->data,
			avFrame->linesize, 0, pic->height,
			pic->avFrame->data, pic->avFrame->linesize);
		
		ModifyFrame(pic, frameNo);
		
		// сохраняем фрейм в файл
		char szFilename[48];
		sprintf(szFilename, "out/frame%03d.png", frameNo);
		pic->savePNG(szFilename);
		
		if ( frameNo % 50 == 0 ) printf("#%i\n", frameNo);
	}
}

void MyInputStream::ModifyFrame(FFCImage *pic, int iFrame)
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
	
	ptr<FFCDemuxer> vin = new FFCDemuxer();
	if ( ! vin->open(fname) ) return -1;
	
	printf("video stream #%d\n", vin->videoStream);
	
	// Allocate video frame
	AVFrame *pFrame = av_frame_alloc();
	
	AVCodecContext *pCodecCtx = vin->videoCodecCtx;
	if ( pCodecCtx == NULL ) return -1;
	
	printf("video size: %dx%d\n", pCodecCtx->width, pCodecCtx->height);
	
	
	
	MyInputStream *videoStream = new MyInputStream;
	videoStream->avCodecCtx = vin->videoCodecCtx;
	// initialize SWS context for software scaling
	videoStream->sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, PIX_FMT_BGRA, SWS_BILINEAR,
		NULL, NULL, NULL);
	
	videoStream->pic = new FFCImage(pCodecCtx->width, pCodecCtx->height);
	videoStream->frameNo = 0;
	vin->bindStream(vin->videoStream, videoStream);
	
	while ( vin->processFrame() ) ;
	
	return 0;
}
