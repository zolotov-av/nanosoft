
#include <ffcairo/ffcdemuxer.h>

#include <stdio.h>

/**
* Конструктор
*/
FFCVideoInput::FFCVideoInput(): avStream(NULL), avDecoder(NULL), scaleCtx(NULL)
{
}

/**
* Деструктор
*/
FFCVideoInput::~FFCVideoInput()
{
	closeScale();
	closeDecoder();
}

/**
* Обработчик присоединения
*
* Автоматически вызывается когда поток присоединяется к демультиплексору
*/
void FFCVideoInput::handleAttach(AVStream *st)
{
	printf("handleAttach()\n");
	avStream = st;
}

/**
* Обработчик отсоединения
*
* Автоматически вызывается когда поток отсоединяется от демультиплексора
*/
void FFCVideoInput::handleDetach()
{
	printf("handleDetach()\n");
	
	closeDecoder();
	
	avStream = NULL;
}

/**
* Открыть кодек
*/
bool FFCVideoInput::openDecoder()
{
	// Find the decoder for the video stream
	AVCodec *codec = avcodec_find_decoder(avStream->codecpar->codec_id);
	if( codec == NULL ) {
		printf("Unsupported codec!\n");
		return false; // Codec not found
	}
	
	avDecoder = avcodec_alloc_context3(codec);
	if ( !avDecoder ) {
		printf("Failed to allocate the decoder context for stream\n");
		return false;
	}
	
	int ret = avcodec_parameters_to_context(avDecoder, avStream->codecpar);
	if (ret < 0) {
		printf("Failed to copy decoder parameters to input decoder context\n");
		return false;
	}
	
	ret = avcodec_open2(avDecoder, codec, 0);
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
	
	avFrame->width  = avDecoder->width;
	avFrame->height = avDecoder->height;
	avFrame->format = avDecoder->pix_fmt;
	
	return true;
}

/**
* Закрыть кодек
*/
void FFCVideoInput::closeDecoder()
{
	// освободить фрейм, указатель будет установлен в NULL
	av_frame_free(&avFrame);
	
	// освободить кодек, указатель будет установлен в NULL
	avcodec_free_context(&avDecoder);
}

/**
* Инициализация маштабирования
*
* При необходимости сменить настройки маштабирования, openScale()
* можно вывызывать без предварительного закрытия через closeScale()
*/
bool FFCVideoInput::openScale(int dstWidth, int dstHeight, AVPixelFormat dstFmt)
{
	scaleCtx = sws_getCachedContext(scaleCtx,
		avFrame->width, avFrame->height, avDecoder->pix_fmt,
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
*
* При необходимости сменить настройки маштабирования, openScale()
* можно вывызывать без предварительного закрытия через closeScale()
*/
bool FFCVideoInput::openScale(ptr<FFCImage> pic)
{
	return openScale(pic->width, pic->height, AV_PIX_FMT_BGRA);
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
* Финализация маштабирования
*/
void FFCVideoInput::closeScale()
{
	if ( scaleCtx )
	{
		sws_freeContext(scaleCtx);
		scaleCtx = NULL;
	}
}

/**
* Обработчик пакета
*/
void FFCVideoInput::handlePacket(AVPacket *packet)
{
	int ret = avcodec_send_packet(avDecoder, packet);
	if ( ret == AVERROR(EAGAIN) )
	{
		printf("avcodec_send_packet() == EAGAIN\n");
		return;
	}
	if ( ret < 0 )
	{
		printf("avcodec_send_packet() failed\n");
		return;
	}
	
	while ( 1 )
	{
		ret = avcodec_receive_frame(avDecoder, avFrame);
		if ( ret < 0 ) break;
		
		handleFrame();
	}
	
	if ( ret == AVERROR(EAGAIN) )
	{
		return;
	}
	else
	{
		printf("avcodec_receive_frame() failed\n");
		return;
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
		if( avFormatCtx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO )
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
	av_packet_unref(&packet);
	
	return true;
}
