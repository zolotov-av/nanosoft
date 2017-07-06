#ifndef FFC_MUXER_H
#define FFC_MUXER_H

#include <nanosoft/object.h>
#include <ffcairo/ffctypes.h>
#include <ffcairo/ffcimage.h>

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
	
	/**
	 * Convert valid timing fields (timestamps / durations) in a packet from
	 * one timebase to another. 
	 */
	void rescale_ts(AVPacket *pkt);
};

/**
 * Класс исходящего видео потока
 */
class FFCVideoOutput: public FFCOutputStream
{
public:
	/**
	 * Фрейм в который надо помещать кадр чтобы записать его в поток
	 */
	AVFrame *avFrame;
	
	/**
	 * Контекст маштабирования и конвертации кадра
	 */
	SwsContext *scaleCtx;
	
	/**
	 * Конструктор
	 */
	FFCVideoOutput(AVStream *st);
	
	/**
	 * Деструктор
	 */
	~FFCVideoOutput();
	
	/**
	 * Открыть кодек
	 */
	bool openCodec(const FFCVideoOptions *opts);
	
	/**
	 * Выделить фрейм
	 */
	bool allocFrame();
	
	/**
	 * Инициализация маштабирования
	 */
	bool initScale(int srcWidth, int srcHeight, AVPixelFormat srcFmt);
	
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
	
	/**
	 * Кодировать кадр
	 */
	bool encode(AVPacket *avpkt, int *got_packet_ptr);
	
	/**
	 * Кодировать кадр с маштабированием
	 */
	bool encode(ptr<FFCImage> pic, AVPacket *avpkt, int *got_packet_ptr);
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
public:
	/**
	 * Контест avFormat
	 */
	AVFormatContext *avFormat;
	
	/**
	 * Номер потока с видео
	 */
	int videoStream;
	
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
	 * Создать видео-поток
	 */
	FFCVideoOutput* createVideoStream(const FFCVideoOptions *opts);
	
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
