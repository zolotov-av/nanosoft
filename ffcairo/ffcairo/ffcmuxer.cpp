
#include <ffcairo/ffcmuxer.h>

#include <stdio.h>

/**
* Конструктор
*/
FFCVideoOutput::FFCVideoOutput(AVStream *st): avStream(st), avEncoder(NULL), avFrame(NULL)
{
}

/**
* Деструктор
*/
FFCVideoOutput::~FFCVideoOutput()
{
	closeEncoder();
}

/**
* Открыть кодек
*/
bool FFCVideoOutput::openEncoder(const FFCVideoOptions *opts)
{
	AVCodec *codec = avcodec_find_encoder(opts->codec_id);
	if ( ! codec )
	{
		printf("encoder not found\n");
		return false;
	}
	
	avEncoder = avcodec_alloc_context3(codec);
	avEncoder->width     = opts->width;
	avEncoder->height    = opts->height;
	avEncoder->pix_fmt   = opts->pix_fmt;
	avEncoder->bit_rate  = opts->bit_rate;
	avStream->time_base   = opts->time_base;
	avEncoder->time_base = avStream->time_base;
	avEncoder->gop_size  = opts->gop_size;
	
	if ( avEncoder->codec_id == AV_CODEC_ID_MPEG2VIDEO )
	{
		/* just for testing, we also add B frames */
		avEncoder->max_b_frames = 2;
	}
	
	if ( avEncoder->codec_id == AV_CODEC_ID_MPEG1VIDEO )
	{
		/* Needed to avoid using macroblocks in which some coeffs overflow.
		 * This does not happen with normal video, it just happens here as
		 * the motion of the chroma plane does not match the luma plane. */
		avEncoder->mb_decision = 2;
	}
	
	int ret = avcodec_open2(avEncoder, codec, NULL);
	if ( ret < 0 )
	{
		printf("avcodec_open2() failed\n");
		return false;
	}
	
	ret = avcodec_parameters_from_context(avStream->codecpar, avEncoder);
	if (ret < 0) {
		printf("Failed to copy encoder parameters to output stream\n");
		return ret;
	}
	
	avFrame = av_frame_alloc();
	if ( !avFrame )
	{
		printf("av_frame_alloc() failed\n");
		return false;
	}
	
	avFrame->width  = opts->width;
	avFrame->height = opts->height;
	avFrame->format = opts->pix_fmt;
	
	/* allocate the buffers for the frame data */
	ret = av_frame_get_buffer(avFrame, 1);
	if ( ret < 0 )
	{
		printf("av_frame_get_buffer() failed\n");
		return false;
	}
	
	return true;
}

/**
* Закрыть кодек
*/
void FFCVideoOutput::closeEncoder()
{
	// освободить фрейм, указатель будет установлен в NULL
	av_frame_free(&avFrame);
	
	// освободить кодек, указатель будет установлен в NULL
	avcodec_free_context(&avEncoder);
}

/**
* Кодировать кадр с маштабированием
*/
bool FFCVideoOutput::encode()
{
	return encode(avFrame);
}

/**
* Кодировать кадр
*/
bool FFCVideoOutput::encode(AVFrame *frame)
{
	int ret = avcodec_send_frame(avEncoder, frame);
	if ( ret == AVERROR(EAGAIN) )
	{
		printf("avcodec_send_frame() == EAGAIN\n");
		return false;
	}
	
	if ( ret < 0 )
	{
		printf("avcodec_send_frame() failed\n");
		return false;
	}
	
	return true;
}

/**
* Получить пакет
*/
bool FFCVideoOutput::recv_packet(AVPacket *pkt, int &got_packet)
{
	int ret = avcodec_receive_packet(avEncoder, pkt);
	if ( ret == 0 )
	{
		got_packet = 1;
		return true;
	}
	
	got_packet = 0;
	
	if ( ret == AVERROR(EAGAIN) )
	{
		//printf("avcodec_receive_packet() == EAGAIN\n");
		return true;
	}
	
	printf("avcodec_receive_packet() failed\n");
	return false;
}

/**
* Конструктор
*/
FFCMuxer::FFCMuxer()
{
}

/**
* Деструктор
*/
FFCMuxer::~FFCMuxer()
{
}

/**
* Создать файл
*
* На самом деле эта функция не создает файла, а только подготавливает
* контекст, который надо будет еще донастроить, в частности создать
* потоки и настроить кодеки, после чего вызвать функцию openFile()
* которая начнет запись файла.
*/
bool FFCMuxer::createFile(const char *fname)
{
	avformat_alloc_output_context2(&avFormat, NULL, NULL, fname);
	if ( !avFormat )
	{
		printf("avformat_alloc_output_context2() failed\n");
		return false;
	}
	
	return true;
}

/**
* Вернуть кодек по умолчанию для аудио
*/
AVCodecID FFCMuxer::defaultAudioCodec()
{
	return avFormat->oformat->audio_codec;
}

/**
* Вернуть кодек по умолчанию для видео
*/
AVCodecID FFCMuxer::defaultVideoCodec()
{
	return avFormat->oformat->video_codec;
}

/**
* Вернуть кодек по умолчанию для субтитров
*/
AVCodecID FFCMuxer::defaultSubtitleCodec()
{
	return avFormat->oformat->subtitle_codec;
}

/**
* Создать поток
*/
AVStream* FFCMuxer::createStream()
{
	AVStream *avStream = avformat_new_stream(avFormat, NULL);
	if ( ! avStream )
	{
		printf("avformat_new_stream() failed\n");
		return NULL;
	}
	
	printf("stream[%d] created\n", avStream->index);
	return avStream;
}

/**
* Создать поток
*/
AVStream* FFCMuxer::createStream(AVCodecID codec_id)
{
	AVCodec *codec = avcodec_find_encoder(codec_id);
	if ( !codec )
	{
		printf("encoder not found\n");
		return NULL;
	}
	
	AVStream *avStream = avformat_new_stream(avFormat, codec);
	if ( ! avStream )
	{
		printf("avformat_new_stream() failed\n");
		return NULL;
	}
	
	printf("stream[%d] created\n", avStream->index);
	return avStream;
}

/**
* Создать видео-поток
*/
FFCVideoOutput* FFCMuxer::createVideoStream(const FFCVideoOptions *opts)
{
	AVStream *avStream = createStream(opts->codec_id);
	if ( !avStream )
	{
		printf("fail to create video stream\n");
		return NULL;
	}
	
	FFCVideoOutput *vo = new FFCVideoOutput(avStream);
	
	return vo;
}

/**
* Открыть файл
*
* Открывает файл и записывает заголовки, перед вызовом должен
* быть создан контекст, потоки, настроены кодеки.
*/
bool FFCMuxer::openFile(const char *fname)
{
	int ret;
	
	/* open the output file, if needed */
	if (!(avFormat->oformat->flags & AVFMT_NOFILE))
	{
		ret = avio_open(&avFormat->pb, fname, AVIO_FLAG_WRITE);
		if (ret < 0)
		{
			fprintf(stderr, "Could not open '%s'\n", fname);
			return false;
		}
	}
	
	ret = avformat_write_header(avFormat, NULL);
	if ( ret < 0 )
	{
		fprintf(stderr, "avformat_write_header() failed\n");
		return false;
	}
	
	return true;
}

/**
* Записать пакет
*/
bool FFCMuxer::writeFrame(AVPacket *pkt)
{
	int ret = av_interleaved_write_frame(avFormat, pkt);
	return ret == 0;
}

/**
* Закрыть файл
*
* Записывает финальные данные и закрывает файл.
*/
bool FFCMuxer::closeFile()
{
	/* Write the trailer, if any. The trailer must be written before you
	 * close the CodecContexts open when you wrote the header; otherwise
	 * av_write_trailer() may try to use memory that was freed on
	 * av_codec_close(). */
	av_write_trailer(avFormat);
	
	avformat_free_context(avFormat);
}
