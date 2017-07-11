#ifndef AVC_HTTP_H
#define AVC_HTTP_H

#include <nanosoft/asyncstream.h>
#include <nanosoft/easyrow.h>
#include <ffcairo/avc_engine.h>

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
		READ_BODY
	} http_state;
	
	bool done;
	
	
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
};

#endif // AVC_HTTP_H
