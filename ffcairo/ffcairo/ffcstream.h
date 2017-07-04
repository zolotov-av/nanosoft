#ifndef FFC_STREAM
#define FFC_STREAM

#include <ffcairo/config.h>

class FFCInputStream
{
friend class FFCInput;
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
 * Класс представляющий входящий видеопоток
 *
 * @note Для упрощения класс не использует приватные члены и не обеспечивает
 *   безопасность. Пользователь может свободно читать и использовать любые поля,
 *   но не должен пытаться перераспределить буфер или менять какие-либо из его
 *   параметров.
 */
class FFCInput
{
private:
	/**
	 * Число потоков
	 */
	int stream_count;
	
	/**
	 * Потоки
	 */
	ffc_input_stream_p *streams;
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
	FFCInput();
	
	/**
	 * Деструктор
	 */
	~FFCInput();
	
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

#endif // FFC_STREAM
