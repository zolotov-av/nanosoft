#ifndef AVC_HTTP_H
#define AVC_HTTP_H

#include <nanosoft/asyncstream.h>
#include <nanosoft/easyrow.h>
#include <ffcairo/avc_engine.h>
#include <ffcairo/ffcimage.h>
#include <ffcairo/scale.h>
#include <ffcairo/ffcmuxer.h>

/**
 * Протокол HTTP для AVCEngine
 *
 * Пока это временный класс для тестирования движка AVCEngine и FFMPEG.
 * Для начала я планирую реализовать здесь серверную часть протокола HTTP,
 * минимально необходимую чтобы vlc мог к нему подключиться и показывать видео
 */
class AVCHttp: public AsyncStream
{
public:
	
	/**
	 * Ссылка на движок
	 */
	AVCEngine *engine;
	
	/**
	 * Буфер для чтения заголовков
	 */
	std::string line;
	
	/**
	 * Начальная строка (неразобранная)
	 */
	std::string method;
	
	/**
	 * Заголовки HTTP
	 */
	EasyRow headers;
	
	enum {
		/**
		 * Чтение стартовой строки
		 */
		READ_METHOD,
		
		/**
		 * Чтение заголовков
		 */
		READ_HEADERS,
		
		/**
		 * Чтение тела
		 */
		READ_BODY,
		
		/**
		 * Потоковое вещание
		 */
		STREAMING
	} http_state;
	
	bool done;
	bool peer_down;
	
	FFCVideoOptions opts;
	
	/**
	 * Хост на котором будет рисоваться видео
	 */
	ptr<FFCImage> pic;
	
	/**
	 * Контест маштабирования
	 */
	ptr<FFCScale> scale;
	
	/**
	 * Мультиплексор
	 */
	ptr<FFCMuxer> muxer;
	
	/**
	 * Видео-поток
	 */
	ptr<FFCVideoOutput> vo;
	
	/**
	 * контекст AVIO
	 */
	AVIOContext *avio_ctx;
	
	/**
	 * Номер фрейма
	 */
	int frameNo;
	
	/**
	 * Конструктор
	 */
	AVCHttp(int afd, AVCEngine *e);
	
	/**
	 * Деструктор
	 */
	virtual ~AVCHttp();
	
protected:
	
	/**
	 * Обработчик прочитанных данных
	 */
	virtual void onRead(const char *data, size_t len);
	
	/**
	 * Обработчик события опустошения выходного буфера
	 *
	 * Вызывается после того как все данные были записаны в файл/сокет
	 */
	virtual void onEmpty();
	
	/**
	 * Пир (peer) закрыл поток.
	 *
	 * Мы уже ничего не можем отправить в ответ,
	 * можем только корректно закрыть соединение с нашей стороны.
	 */
	virtual void onPeerDown();
	
public:
	
	void write(const std::string &s);
	
	/**
	 * Инициализация стриминга
	 */
	void initStreaming();
	
	/**
	 * Обработчик записи пакета в поток
	 */
	static int write_packet(void *opaque, uint8_t *buf, int buf_size);
	
	/**
	 * Временный таймер
	 */
	void onTimer();
	
	/**
	 * прервать передачу
	 */
	void endStreaming();
	
	/**
	 * Отрисовать фрейм
	 */
	void DrawPic();
};

#endif // AVC_HTTP_H
