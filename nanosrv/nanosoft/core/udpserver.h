#ifndef NANOSOFT_UDPSERVER_H
#define NANOSOFT_UDPSERVER_H

#include <nanosoft/core/netobject.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>

/**
* Базовый класс для асинхронных серверов
*/
class UDPServer: public NetObject
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
	
	/**
	* Принять входящее соединение
	*/
	virtual void onRead() = 0;
	
public:
	/**
	* Конструктор
	*/
	UDPServer();
	
	/**
	* Деструктор
	*/
	~UDPServer();
	
	/**
	* Подключиться к порту
	*/
	bool bind(int port);
	
	/**
	* Закрыть сокет
	*/
	void close();
};

#endif // NANOSOFT_UDPSERVER_H
