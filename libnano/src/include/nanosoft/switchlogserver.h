#ifndef SWITCH_LOG_SERVER_H
#define SWITCH_LOG_SERVER_H

#include <nanosoft/asyncudpserver.h>
#include <db.h>
#include <pcre.h>

/**
* Syslog-сервер для мониторинга коммутаторов
*/
class SwitchLogServer: public AsyncUDPServer
{
protected:
	/**
	* Регулярное выражение для парсинга сообщений
	*/
	pcre *re;
	
	/**
	* Обработать входящее сообщение
	* 
	* @param ip адрес источника
	* @param port порт источника
	* @param data пакет данных
	* @param len размер пакета данных
	*/
	virtual void onRead(const char *ip, int port, const char *data, size_t len);
	
	/**
	* Сигнал завершения работы
	*
	* Сервер решил закрыть соединение, здесь ещё есть время
	* корректно попрощаться с пиром (peer).
	*/
	virtual void onTerminate();
	
	/**
	* Обработчик включения порта
	*
	* @param sw_ip IP-адрес коммутатора
	* @param port порт на коммутаторе
	* @param speed скорость на которой включился порт
	*/
	void handlePortUp(const char *sw_ip, int port, const char *speed);
	
	/**
	* Обработчик выключения порта
	*
	* @param sw_ip IP-адрес коммутатора
	* @param port порт на коммутаторе
	*/
	void handlePortDown(const char *sw_ip, int port);
	
	/**
	* Обработчик обнаружения петли на порту
	*
	* @param sw_ip IP-адрес коммутатора
	* @param port порт на коммутаторе
	*/
	void handlePortLoop(const char *sw_ip, int port);
public:
	/**
	* База данных
	*/
	DB db;
	
	/**
	* Конструктор
	*/
	SwitchLogServer();
	
	/**
	* Деструктор
	*/
	~SwitchLogServer();
};

#endif // SWITCH_LOG_SERVER_H
