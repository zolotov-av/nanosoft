#ifndef NANOSOFT_ASYNCSERVER_H
#define NANOSOFT_ASYNCSERVER_H

#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <nanosoft/asyncobject.h>

/**
* Базовый класс для асинхронных серверов
*/
class AsyncServer: public AsyncObject
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
	AsyncServer();
	
	/**
	* Деструктор
	*/
	~AsyncServer();
	
	/**
	* Подключиться к порту
	*/
	bool bind(int port);
	
	/**
	* Начать слушать сокет
	*/
	bool listen(int backlog);
	
	/**
	* Принять соединение
	*/
	int accept();
	
	/**
	* Событие ошибки
	*
	* Вызывается в случае возникновения какой-либо ошибки
	*/
	virtual void onError(const char *message);
	
	/**
	* Принять входящее соединение
	*/
	virtual AsyncObject* onAccept() = 0;
};

#endif // NANOSOFT_ASYNCSERVER_H