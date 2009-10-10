#ifndef NANOSOFT_ASYNCSTREAM_H
#define NANOSOFT_ASYNCSTREAM_H

#include <sys/types.h>

/**
* Класс для асинхронной работы потоками
*/
class AsyncStream
{
friend class NetDaemon;
private:
	/**
	* Файловый дескриптор
	*/
	int fd;
	
	/**
	* Конструктор копий
	*
	* Не ищите его реализации, его нет и не надо.
	* Просто блокируем конкструктор копий по умолчанию
	*/
	AsyncStream(const AsyncStream &);
	
	/**
	* Оператор присваивания
	*
	* Блокируем аналогично конструктору копий
	*/
	AsyncStream& operator = (const AsyncStream &);
	
protected:
	
	/**
	* Обработка системной ошибки
	*/
	void stderror();
	
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
	* Событие ошибки
	*
	* Вызывается в случае возникновения какой-либо ошибки.
	* По умолчанию выводит все ошибки в stderr
	*/
	virtual void onShutdown() = 0;
};

#endif // NANOSOFT_ASYNCSTREAM_H
