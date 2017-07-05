
#include <ffcairo/ffcmuxer.h>

#include <stdio.h>

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
