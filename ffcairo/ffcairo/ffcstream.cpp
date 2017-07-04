
#include <ffcairo/ffcstream.h>

#include <stdio.h>

/**
* Конструктор
*/
FFCInput::FFCInput(): avFormatCtx(NULL), videoStream(-1), videoCodecCtx(NULL), audioStream(-1)
{
}

/**
* Деструктор
*/
FFCInput::~FFCInput()
{
}

/**
* Найти видео-поток
*/
void FFCInput::findVideoStream()
{
	videoStream = -1;
	int count = avFormatCtx->nb_streams;
	for(int i=0; i<count; i++)
	{
		if( avFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO )
		{
			videoStream = i;
			break;
		}
	}
	
	if(videoStream==-1)
	{
		fprintf(stderr, "video stream not found\n");
		return;
	}
	
	// Get a pointer to the codec context for the video stream
	AVCodecContext *pCodecCtx = avFormatCtx->streams[videoStream]->codec;
	if ( pCodecCtx == NULL )
	{
		fprintf(stderr, "pCodecCtx(video) is NULL\n");
		return;
	}
	
	// Find the decoder for the video stream
	AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if( pCodec == NULL ) {
		printf("Unsupported codec!\n");
		return; // Codec not found
	}
	
	// Copy context
	videoCodecCtx = avcodec_alloc_context3(pCodec);
	if( avcodec_copy_context(videoCodecCtx, pCodecCtx) != 0 )
	{
		printf("Couldn't copy codec context");
		return; // Error copying codec context
	}
	
	//AVDictionary *opts = NULL;
	//av_dict_set(&opts, "refcounted_frames", "1", 0);
	
	// Open codec
	if( avcodec_open2(videoCodecCtx, pCodec, 0) < 0 ) {
		printf("Could not open codec\n");
		return; // Could not open codec
	}

}

/**
* Открыть поток
*
* Это может быть файл или URL, любой поддеживаемый FFMPEG-ом ресурс
*/
bool FFCInput::open(const char *uri)
{
	/* 
	 * TODO something...
	 * Здесь avFormatCtx должен быть NULL, а что если он не-NULL?
	 * непонятно, можно ли повторно вызывать avformat_open_input()
	 * или надо пересоздавать контекст...
	 */
	
	// Открываем файл/URL
	if ( avformat_open_input(&avFormatCtx, uri, NULL, 0) != 0 )
	{
		fprintf(stderr, "avformat_open_input(%s) fault\n", uri);
		return false;
	}
	
	// Извлечь информацию о потоке
	if( avformat_find_stream_info(avFormatCtx, NULL) < 0 )
	{
		fprintf(stderr, "avformat_find_stream_info(%s) failed\n", uri);
		return false;
	}
	
	// Вывести информацию о потоке на стандартный вывод (необязательно)
	av_dump_format(avFormatCtx, 0, uri, 0);
	
	findVideoStream();
	
	printf("Поток[%s] успешно открыт\n", uri);
	
	return true;
}
