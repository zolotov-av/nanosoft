#ifndef FFC_DEMUXER_H
#define FFC_DEMUXER_H

#include <ffcairo/ffctypes.h>
#include <nanosoft/object.h>

/**
 * Абстрактный класс входного потока
 *
 * Это может быть и видео и аудио и субтитры и что угодно
 */
class FFCInputStream: public Object
{
friend class FFCDemuxer;
protected:
	/**
	 * Обработчик пакета
	 */
	virtual void handlePacket(AVPacket *packet) = 0;
};

typedef FFCInputStream *ffc_input_stream_p;

class FFCDecodedInput: public FFCInputStream
{
public:
	/**
	 * Контекст кодека
	 */
	AVCodecContext *avCodecCtx;
	
	/**
	 * Фрейм (кадр)
	 */
	AVFrame *avFrame;
	
	/**
	 * Конструктор
	 */
	FFCDecodedInput();
	
	/**
	 * Деструктор
	 */
	~FFCDecodedInput();
protected:
	/**
	 * Обработчик пакета
	 */
	virtual void handlePacket(AVPacket *packet);
	
	/**
	 * Обработчик фрейма
	 */
	virtual void handleFrame() = 0;
};

/**
 * Демультиплексор
 *
 * Позволяет читать и декодировать медиа
 *
 * @note Для упрощения класс не использует приватные члены и не обеспечивает
 *   безопасность. Пользователь может свободно читать и использовать любые поля,
 *   но не должен пытаться перераспределить буфер или менять какие-либо из его
 *   параметров.
 */
class FFCDemuxer: public Object
{
private:
	/**
	 * Число потоков
	 */
	int stream_count;
	
	/**
	 * Потоки
	 */
	ptr<FFCInputStream> *streams;
public:
	/**
	 * Контест avFormat
	 */
	AVFormatContext *avFormatCtx;
	
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
	FFCDemuxer();
	
	/**
	 * Деструктор
	 */
	~FFCDemuxer();
	
protected:
	/**
	 * Найти видео-поток
	 */
	void findVideoStream();
	
public:
	/**
	 * Открыть поток
	 *
	 * Это может быть файл или URL, любой поддеживаемый FFMPEG-ом ресурс
	 */
	bool open(const char *uri);
	
	/**
	 * Присоединить приемник потока
	 */
	void bindStream(int i, FFCInputStream *s);
	
	/**
	 * Обработать фрейм
	 */
	bool processFrame();
};

#endif // FFC_DEMUXER_H
