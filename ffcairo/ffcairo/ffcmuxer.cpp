
#include <ffcairo/ffcmuxer.h>

#include <stdio.h>

/**
* Конструктор
*/
FFCOutputStream::FFCOutputStream(AVStream *st): avStream(st), avCodecCtx(NULL)
{
	if ( st ) avCodecCtx = st->codec;
}

/**
* Деструктор
*/
FFCOutputStream::~FFCOutputStream()
{
}

/**
* Конструктор
*/
FFCVideoOutput::FFCVideoOutput(AVStream *st): FFCOutputStream(st)
{
}

/**
* Деструктор
*/
FFCVideoOutput::~FFCVideoOutput()
{
}

/**
* Установить параметры картинки
*/
void FFCVideoOutput::setImageOptions(int width, int height,  AVPixelFormat fmt)
{
	avCodecCtx->width    = width;
	avCodecCtx->height   = height;
	avCodecCtx->pix_fmt  = fmt;
}

/**
* Установить параметры видео
*/
void FFCVideoOutput::setVideoOptions(int64_t bit_rate, AVRational time_base, int gop_size)
{
	avCodecCtx->bit_rate = bit_rate;
	/* timebase: This is the fundamental unit of time (in seconds) in terms
	 * of which frame timestamps are represented. For fixed-fps content,
	 * timebase should be 1/framerate and timestamp increments should be
	 * identical to 1. */
	avStream->time_base = time_base;
	avCodecCtx->time_base = avStream->time_base;
	avCodecCtx->gop_size = gop_size;
	
	if ( avCodecCtx->codec_id == AV_CODEC_ID_MPEG2VIDEO )
	{
		/* just for testing, we also add B frames */
		avCodecCtx->max_b_frames = 2;
	}
	if ( avCodecCtx->codec_id == AV_CODEC_ID_MPEG1VIDEO )
	{
		/* Needed to avoid using macroblocks in which some coeffs overflow.
		 * This does not happen with normal video, it just happens here as
		 * the motion of the chroma plane does not match the luma plane. */
		avCodecCtx->mb_decision = 2;
	}
}

/**
* Выделить фрейм
*/
bool FFCVideoOutput::allocFrame()
{
	avFrame = av_frame_alloc();
	if ( !avFrame )
	{
		printf("av_frame_alloc() failed\n");
		return false;
	}
	avFrame->format = avCodecCtx->pix_fmt;
	avFrame->width = avCodecCtx->width;
	avFrame->height = avCodecCtx->height;
	
	/* allocate the buffers for the frame data */
	// TODO разобраться в форматом пикселей, почему два типа и как их конвертировать!!!
	int ret = avpicture_alloc((AVPicture *)avFrame, avCodecCtx->pix_fmt, avFrame->width, avFrame->height);
	if ( ret < 0 )
	{
		printf("avpicture_alloc() failed\n");
		return false;
	}
	
	return true;
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
		// TODO нужно ли как-то освободжать AVCodec* и как это делать?
		return NULL;
	}
	
	printf("stream[%d] created\n", avStream->index);
	return avStream;
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
