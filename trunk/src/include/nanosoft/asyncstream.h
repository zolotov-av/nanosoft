#ifndef NANOSOFT_ASYNCSTREAM_H
#define NANOSOFT_ASYNCSTREAM_H

#include <sys/types.h>
#include <nanosoft/asyncobject.h>

/**
* Класс для асинхронной работы потоками
*/
class AsyncStream: public AsyncObject
{
protected:
	
	/**
	* Вернуть маску ожидаемых событий
	*/
	virtual uint32_t getEventsMask();
	
	/**
	* Обработчик события
	*/
	virtual void onEvent(uint32_t events);
	
public:
	/**
	* Конструктор
	*/
	AsyncStream(int afd);
	
	/**
	* Деструктор
	*/
	virtual ~AsyncStream();
	
	/**
	* Неблокирующее чтение из потока
	*/
	ssize_t read(void *buf, size_t count);
	
	/**
	* Неблокирующая запись в поток
	*/
	ssize_t write(const void *buf, size_t count);
	
	/**
	* Приостановить чтение из потока
	*/
	bool suspend();
	
	/**
	* Возобновить чтение из потока
	*/
	bool resume();
	
	/**
	* Закрыть поток
	*/
	void close();
	
	/**
	* Событие готовности к чтению
	*
	* Вызывается когда в потоке есть данные,
	* которые можно прочитать без блокирования
	*/
	virtual void onRead() = 0;
	
	/**
	* Событие готовности к записи
	*
	* Вызывается, когда в поток готов принять
	* данные для записи без блокировки
	*/
	virtual void onWrite() = 0;
	
	/**
	* Событие ошибки
	*
	* Вызывается в случае возникновения какой-либо ошибки
	*/
	virtual void onError(const char *message);
	
	/**
	* Событие закрытия потока
	*
	* Вызывается в случае достижения конца файла
	* или если противоположный конец закрыл поток
	*/
	virtual void onShutdown() = 0;
};

#endif // NANOSOFT_ASYNCSTREAM_H
