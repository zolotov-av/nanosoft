
#include <ffcairo/ffcdemuxer.h>

#include <stdio.h>

/**
* Конструктор
*/
FFCInputStream::FFCInputStream(): avStream(NULL), avCodecCtx(NULL)
{
}

/**
* Деструктор
*/
FFCInputStream::~FFCInputStream()
{
}

/**
* Обработчик присоединения
*
* Автоматически вызывается когда поток присоединяется к демультиплексору
*/
void FFCInputStream::handleAttach(AVStream *st)
{
	printf("handleAttach()\n");
	avStream = st;
	avCodecCtx = st->codec;
}

/**
* Обработчик отсоединения
*
* Автоматически вызывается когда поток отсоединяется от демультиплексора
*/
void FFCInputStream::handleDetach()
{
	printf("handleDetach()\n");
	avStream = NULL;
	avCodecCtx = NULL;
}

/**
* Конструктор
*/
FFCVideoInput::FFCVideoInput(): scaleCtx(NULL)
{
	//avFrame = av_frame_alloc();
}

/**
* Деструктор
*/
FFCVideoInput::~FFCVideoInput()
{
	av_frame_free(&avFrame);
}

/**
* Открыть кодек
*/
bool FFCVideoInput::openDecoder()
{
	// Find the decoder for the video stream
	AVCodec *pCodec = avcodec_find_decoder(avCodecCtx->codec_id);
	if( pCodec == NULL ) {
		printf("Unsupported codec!\n");
		return false; // Codec not found
	}
	
	/*
	// Copy context
	AVCodecContext *videoCodecCtx = avcodec_alloc_context3(pCodec);
	if( avcodec_copy_context(videoCodecCtx, avCodecCtx) != 0 )
	{
		printf("Couldn't copy codec context");
		return false; // Error copying codec context
	}
	
	//AVDictionary *opts = NULL;
	//av_dict_set(&opts, "refcounted_frames", "1", 0);
	*/
	
	// Open codec
	int ret = avcodec_open2(avCodecCtx, pCodec, 0);
	if( ret < 0 ) {
		printf("Could not open codec\n");
		return false; // Could not open codec
	}
	
	avFrame = av_frame_alloc();
	if ( !avFrame )
	{
		printf("av_frame_alloc() failed\n");
		return false;
	}
	
	avFrame->width  = avCodecCtx->width;
	avFrame->height = avCodecCtx->height;
	avFrame->format = avCodecCtx->pix_fmt;
	
	/* allocate the buffers for the frame data */
	// TODO выделять буфен не обязательно, но надо разобраться что эффективнее
	ret = avpicture_alloc((AVPicture *)avFrame, avCodecCtx->pix_fmt, avCodecCtx->width, avCodecCtx->height);
	if ( ret < 0 )
	{
		printf("avpicture_alloc() failed\n");
		return false;
	}
	
	return true;
}

/**
* Инициализация маштабирования
*/
bool FFCVideoInput::initScale(int dstWidth, int dstHeight, AVPixelFormat dstFmt)
{
	scaleCtx = sws_getContext(avFrame->width, avFrame->height, avCodecCtx->pix_fmt,
		dstWidth, dstHeight, dstFmt,
		SWS_BILINEAR, NULL, NULL, NULL);
	
	if ( ! scaleCtx )
	{
		printf("sws_getContext(%d, %d) failed\n", dstWidth, dstHeight);
		return true;
	}
	
	return true;
}

/**
* Инициализация маштабирования
*/
bool FFCVideoInput::initScale(ptr<FFCImage> pic)
{
	return initScale(pic->width, pic->height, PIX_FMT_BGRA);
}

/**
* Маштабировать картику
*/
void FFCVideoInput::scale(AVFrame *pFrame)
{
	// TODO check params
	sws_scale(scaleCtx, avFrame->data, avFrame->linesize,
		0, avFrame->height, pFrame->data, pFrame->linesize);
}

/**
* Маштабировать картинку
*/
void FFCVideoInput::scale(ptr<FFCImage> pic)
{
	// TODO check params
	sws_scale(scaleCtx, avFrame->data, avFrame->linesize,
		0, avFrame->height, pic->avFrame->data, pic->avFrame->linesize);
}

/**
* Обработчик пакета
*/
void FFCVideoInput::handlePacket(AVPacket *packet)
{
	// Decode video frame
	int frameFinished = 0;
	avcodec_decode_video2(avCodecCtx, avFrame, &frameFinished, packet);
	
	// Did we get a video frame?
	if(frameFinished)
	{
		handleFrame();
	}
}

/**
* Конструктор
*/
FFCDemuxer::FFCDemuxer(): avFormatCtx(NULL), stream_count(0), streams(NULL)
{
}

/**
* Деструктор
*/
FFCDemuxer::~FFCDemuxer()
{
	if ( streams ) delete [] streams;
}

/**
* Найти видео-поток
*
* Возвращает ID потока или -1 если не найден
*/
int FFCDemuxer::findVideoStream()
{
	int videoStream = -1;
	printf("stream's count: %d\n", stream_count);
	for(int i=0; i<stream_count; i++)
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
		return -1;
	}
	
	return videoStream;
}

/**
* Открыть поток
*
* Это может быть файл или URL, любой поддеживаемый FFMPEG-ом ресурс
*/
bool FFCDemuxer::open(const char *uri)
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
	
	stream_count = avFormatCtx->nb_streams;
	streams = new ptr<FFCInputStream>[stream_count];
	for(int i = 0; i < stream_count; i++) streams[i] = NULL;
	
	printf("Поток[%s] успешно открыт\n", uri);
	
	return true;
}

/**
* Присоединить приемник потока
*/
void FFCDemuxer::bindStream(int i, ptr<FFCInputStream> st)
{
	if ( i >= 0 && i < stream_count )
	{
		if ( streams[i].getObject() )
		{
			streams[i]->handleDetach();
		}
		
		streams[i] = st;
		
		if ( st.getObject() )
		{
			st->handleAttach(avFormatCtx->streams[i]);
		}
	}
}

/**
* Обработать фрейм
*/
bool FFCDemuxer::readFrame()
{
	AVPacket packet;
	
	int r = av_read_frame(avFormatCtx, &packet);
	if ( r < 0 )
	{
		// конец файла или ошибка
		return false;
	}
	
	int id = packet.stream_index;
	if ( id >= 0 && id < stream_count )
	{
		if ( streams[id].getObject() )
		{
			streams[id]->handlePacket(&packet);
		}
	}
	
	// Free the packet that was allocated by av_read_frame
	av_free_packet(&packet);
	
	return true;
}
