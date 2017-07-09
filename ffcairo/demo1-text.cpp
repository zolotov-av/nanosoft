/****************************************************************************

Демо-программа №1

Демонстрирует возможности модификации изображения при перекодировании

****************************************************************************/

#include <ffcairo/config.h>
#include <ffcairo/ffcimage.h>
#include <ffcairo/ffcdemuxer.h>

class MyInputStream: public FFCVideoInput
{
public:
	int frameNo;
	ptr<FFCImage> pic;
	
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
		scale(pic);
		
		ModifyFrame(pic.getObject(), frameNo);
		
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
	const char *fname = argc > 1 ? argv[1] : "test.avi";
	printf("input filename = %s\n", fname);
	
	// INIT
	av_register_all();
	avformat_network_init();
	
	ptr<FFCDemuxer> demux = new FFCDemuxer();
	
	// открыть файл
	if ( ! demux->open(fname) )
	{
		printf("fail to open file\n");
		return -1;
	}
	
	// найти видео-поток
	int video_stream = demux->findVideoStream();
	if ( video_stream < 0 )
	{
		printf("fail to find video stream\n");
		return -1;
	}
	printf("video stream #%d\n", video_stream);
	
	// присоединить обработчик потока
	ptr<MyInputStream> videoStream = new MyInputStream;
	demux->bindStream(video_stream, videoStream);
	
	int width = videoStream->avStream->codecpar->width;
	int height = videoStream->avStream->codecpar->height;
	printf("video size: %dx%d\n", width, height);
	
	// открыть декодер видео
	if ( ! videoStream->openDecoder() )
	{
		printf("stream[%d] openDecoder() failed\n", video_stream);
		return -1;
	}
	
	videoStream->pic = FFCImage::createImage(width, height);
	if ( videoStream->pic.getObject() == NULL )
	{
		printf("fail to create FFCImage\n");
		return -1;
	}
	videoStream->initScale(videoStream->pic);
	videoStream->frameNo = 0;
	
	while ( demux->readFrame() ) ;
	
	return 0;
}
