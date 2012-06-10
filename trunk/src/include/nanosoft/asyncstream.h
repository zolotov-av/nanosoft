#ifndef NANOSOFT_ASYNCSTREAM_H
#define NANOSOFT_ASYNCSTREAM_H

#include <sys/types.h>
#include <nanosoft/asyncobject.h>
#include <nanosoft/netdaemon.h>

/**
* Класс для асинхронной работы потоками
*/
class AsyncStream: public AsyncObject
{
private:
	/**
	* Флаги
	*/
	int flags;
	
	/**
	* Обработка поступивших данных
	*/
	void handleRead();
	
	/**
	* Отправка накопленных данных
	*/
	void handleWrite();
	
protected:
	
	/**
	* Вернуть маску ожидаемых событий
	*/
	virtual uint32_t getEventsMask();
	
	/**
	* Обработчик события
	*/
	virtual void onEvent(uint32_t events);
	
	/**
	* Обработчик прочитанных данных
	*/
	virtual void onRead(const char *data, size_t len) = 0;
	
	/**
	* Пир (peer) закрыл поток.
	*
	* Мы уже ничего не можем отправить в ответ,
	* можем только корректно закрыть соединение с нашей стороны.
	*/
	virtual void onPeerDown() = 0;
	
public:
	
	/**
	* Для shutdown()
	*/
	enum { READ = 1, WRITE = 2 };
	
	/**
	* Конструктор
	*/
	AsyncStream(int afd);
	
	/**
	* Деструктор
	*/
	virtual ~AsyncStream();
	
	/**
	* Проверить поддерживается ли компрессия
	* @return TRUE - компрессия поддерживается, FALSE - компрессия не поддерживается
	*/
	bool canCompression();
	
	/**
	* Проверить поддерживается ли компрессия конкретным методом
	* @param method метод компрессии
	* @return TRUE - компрессия поддерживается, FALSE - компрессия не поддерживается
	*/
	bool canCompression(const char *method);
	
	/**
	* Вернуть список поддерживаемых методов компрессии
	*/
	const compression_method_t* getCompressionMethods();
	
	/**
	* Вернуть флаг компрессии
	* @return TRUE - компрессия включена, FALSE - компрессия отключена
	*/
	bool isCompressionEnable();
	
	/**
	* Вернуть текущий метод компрессии
	* @return имя метода компрессии или NULL если компрессия не включена
	*/
	compression_method_t getCompressionMethod();
	
	/**
	* Включить компрессию
	* @param method метод компрессии
	* @return TRUE - компрессия включена, FALSE - компрессия не включена
	*/
	bool enableCompression(compression_method_t method);
	
	/**
	* Отключить компрессию
	* @return TRUE - компрессия отключена, FALSE - произошла ошибка
	*/
	bool disableCompression();
	
	/**
	* Записать данные
	*
	* Данные записываются сначала в файловый буфер и только потом отправляются.
	* Для обеспечения целостности переданный блок либо записывается целиком
	* и функция возвращает TRUE, либо ничего не записывается и функция
	* возвращает FALSE
	*
	* @param data указатель на данные
	* @param len размер данных
	* @return TRUE данные приняты, FALSE данные не приняты - нет места
	*/
	bool put(const char *data, size_t len);
	
	/**
	* Завершить чтение/запись
	* @note только для сокетов
	*/
	bool shutdown(int how);
	
	/**
	* Закрыть поток
	*/
	void close();
};

#endif // NANOSOFT_ASYNCSTREAM_H
