#ifndef FFC_STREAM
#define FFC_STREAM

#include <ffcairo/config.h>

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
};

#endif // FFC_STREAM
