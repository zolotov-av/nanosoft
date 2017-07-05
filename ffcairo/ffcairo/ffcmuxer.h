#ifndef FFC_MUXER_H
#define FFC_MUXER_H

#include <ffcairo/config.h>
#include <nanosoft/object.h>

/**
 * Абстрактный класс исходящего потока
 *
 * Это может быть и видео и аудио и субтитры и что угодно
 */
class FFCOutputStream: public Object
{
public:
	/**
	 * Ссылка на поток
	 */
	AVStream *avStream;
	
	/**
	 * Ссылка на контекст кодека
	 */
	AVCodecContext *avCodecCtx;
	
	/**
	 * Конструктор
	 */
	FFCOutputStream(AVStream *st);
	
	/**
	 * Деструктор
	 */
	~FFCOutputStream();
};

class FFCVideoOutput: public FFCOutputStream
{
public:
	/**
	 * Фрейм в который надо помещать кадр чтобы записать его в поток
	 */
	AVFrame *avFrame;
	
	/**
	 * Конструктор
	 */
	FFCVideoOutput(AVStream *st);
	
	/**
	 * Деструктор
	 */
	~FFCVideoOutput();
	
	/**
	 * Установить параметры картинки
	 */
	void setImageOptions(int width, int height,  AVPixelFormat fmt);
	
	/**
	 * Установить параметры видео
	 */
	void setVideoOptions(int64_t bit_rate, AVRational time_base, int gop_size = 12);
	
	/**
	 * Выделить фрейм
	 */
	bool allocFrame();
};

/**
 * Класс представляющий исходящий видеопоток - поток который записывается
 * в файл или передается по сети на другой хост
 *
 * @note Для упрощения класс не использует приватные члены и не обеспечивает
 *   безопасность. Пользователь может свободно читать и использовать любые поля,
 *   но не должен пытаться перераспределить буфер или менять какие-либо из его
 *   параметров.
 */
class FFCMuxer: public Object
{
private:
	/**
	 * Число потоков
	 */
	int stream_count;
	
	/**
	 * Потоки
	 */
	ptr<FFCOutputStream> *streams;
public:
	/**
	 * Контест avFormat
	 */
	AVFormatContext *avFormat;
	
	/**
	 * Номер потока с видео
	 */
	int videoStream;
	
	AVCodecContext *videoCodecCtx;
	
	/**
	 * Номер потока с аудио
	 */
	int audioStream;
	
	/**
	 * Конструктор
	 */
	FFCMuxer();
	
	/**
	 * Деструктор
	 */
	~FFCMuxer();
	
public:
	/**
	 * Создать файл
	 *
	 * На самом деле эта функция не создает файла, а только подготавливает
	 * контекст, который надо будет еще донастроить, в частности создать
	 * потоки и настроить кодеки, после чего вызвать функцию openFile()
	 * которая начнет запись файла.
	 */
	bool createFile(const char *fname);
	
	/**
	 * Вернуть кодек по умолчанию для аудио
	 */
	AVCodecID defaultAudioCodec();
	
	/**
	 * Вернуть кодек по умолчанию для видео
	 */
	AVCodecID defaultVideoCodec();
	
	/**
	 * Вернуть кодек по умолчанию для субтитров
	 */
 	AVCodecID defaultSubtitleCodec();
	
	/**
	 * Создать поток
	 */
	AVStream* createStream(AVCodecID codec_id);
	
	/**
	 * Открыть файл
	 *
	 * Открывает файл и записывает заголовки, перед вызовом должен
	 * быть создан контекст, потоки, настроены кодеки.
	 */
	bool openFile(const char *fname);
	
	/**
	 * Записать пакет
	 */
	bool writeFrame(AVPacket *pkt);
	
	/**
	 * Закрыть файл
	 *
	 * Записывает финальные данные и закрывает файл.
	 */
	bool closeFile();
};

#endif // FFC_MUXER_H
