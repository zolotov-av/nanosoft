#ifndef FFC_DEMUXER_H
#define FFC_DEMUXER_H

#include <ffcairo/ffctypes.h>
#include <ffcairo/ffcimage.h>
#include <nanosoft/object.h>

/**
 * Абстрактный класс входного потока
 *
 * Это может быть и видео и аудио и субтитры и что угодно
 */
class FFCInputStream: public Object
{
friend class FFCDemuxer;
public:
	/**
	 * Ссылка на поток
	 */
	AVStream *avStream;
	
	/**
	 * Контекст кодека
	 */
	AVCodecContext *avCodecCtx;
	
	/**
	 * Конструктор
	 */
	FFCInputStream();
	
	/**
	 * Деструктор
	 */
	~FFCInputStream();
	
protected:
	/**
	 * Обработчик присоединения
	 *
	 * Автоматически вызывается когда поток присоединяется к демультиплексору
	 */
	virtual void handleAttach(AVStream *st);
	
	/**
	 * Обработчик отсоединения
	 *
	 * Автоматически вызывается когда поток отсоединяется от демультиплексора
	 */
	virtual void handleDetach();
	
	/**
	 * Обработчик пакета
	 */
	virtual void handlePacket(AVPacket *packet) = 0;
};

/**
 * Класс входящего видео потока
 *
 * Данный класс позволяет декодировать поток и получать отдельные кадры
 */
class FFCVideoInput: public FFCInputStream
{
public:
	/**
	 * Фрейм (кадр)
	 */
	AVFrame *avFrame;
	
	/**
	 * Контекст маштабирования и конвертации кадра
	 */
	SwsContext *scaleCtx;
	
	/**
	 * Конструктор
	 */
	FFCVideoInput();
	
	/**
	 * Деструктор
	 */
	~FFCVideoInput();
	
	/**
	 * Открыть кодек
	 */
	bool openDecoder();
	
	/**
	 * Инициализация маштабирования
	 */
	bool initScale(int dstWidth, int dstHeight, AVPixelFormat dstFmt);
	
	/**
	 * Инициализация маштабирования
	 */
	bool initScale(ptr<FFCImage> pic);
	
	/**
	 * Маштабировать картику
	 */
	void scale(AVFrame *pFrame);
	
	/**
	 * Маштабировать картинку
	 */
	void scale(ptr<FFCImage> pic);
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
	 * Конструктор
	 */
	FFCDemuxer();
	
	/**
	 * Деструктор
	 */
	~FFCDemuxer();
	
public:
	/**
	 * Открыть поток
	 *
	 * Это может быть файл или URL, любой поддеживаемый FFMPEG-ом ресурс
	 */
	bool open(const char *uri);
	
	/**
	 * Найти видео-поток
	 *
	 * Возвращает ID потока или -1 если не найден
	 */
	int findVideoStream();
	
	/**
	 * Присоединить приемник потока
	 */
	void bindStream(int i, ptr<FFCInputStream> st);
	
	/**
	 * Обработать фрейм
	 */
	bool readFrame();
};

#endif // FFC_DEMUXER_H