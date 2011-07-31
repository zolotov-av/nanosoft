#ifndef SYSLOG_SERVER_H
#define SYSLOG_SERVER_H

#include <nanosoft/core/udpserver.h>

#include <time.h>

/**
* Сервер syslog
*/
class SyslogServer: public UDPServer
{
protected:
	/**
	* Принять входящее соединение
	*/
	virtual void onRead();
	
	/**
	* Обработка очередного log-сообщения
	*/
	virtual void onMessage(const char *ip, const char *message) = 0;
	
	/**
	* Сигнал завершения работы
	*
	* Сервер решил закрыть соединение, здесь ещё есть время
	* корректно попрощаться с пиром (peer).
	*/
	virtual void onTerminate();
public:
	/**
	* Конструктор
	*/
	SyslogServer();
	
	/**
	* Деструктор
	*/
	~SyslogServer();
};

#endif // SYSLOG_SERVER_H
