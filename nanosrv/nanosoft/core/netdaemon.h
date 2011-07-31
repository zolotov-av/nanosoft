#ifndef NANOSOFT_NETDAEMON_H
#define NANOSOFT_NETDAEMON_H

#include <nanosoft/core/object.h>
#include <nanosoft/core/netobject.h>

#include <sys/epoll.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

/**
* Callback таймера
*/
typedef void (*timer_callback_t)();

/**
* Главный класс сетевого демона
*/
class NetDaemon
{
private:
	/**
	* Файловый дескриптор epoll
	*/
	int epoll;
	
	/**
	* Максимальное число обслуживаемых объектов
	*/
	size_t limit;
	
	/**
	* Текущее число объектов
	*/
	size_t count;
	
	/**
	* Карта объектов (fd -> AsyncObject)
	*/
	ptr<NetObject> *objects;
	
	/**
	* Состояние демона
	*/
	enum daemon_state_t {
		/**
		* Поток не запущен
		*/
		INACTIVE,
		
		/**
		* Поток в обычном рабочем цикле
		*/
		ACTIVE
	};
	
	daemon_state_t state;
	
	/**
	* Текущее время в секундах (Unix time)
	*/
	time_t tm;
	
	/**
	* Обработка системной ошибки
	*/
	void stderror();
	
	/**
	* Возобновить работу с асинхронным объектом
	*/
	bool resetObject(ptr<NetObject> &object);
public:
	/**
	* Установка таймера (вызывается примерно раз в секунду)
	*/
	timer_callback_t timer;
	
	/**
	* Конструктор демона
	* @param aLimit максимальное число одновременных виртуальных потоков
	*/
	NetDaemon(int aLimit);
	
	/**
	* Деструктор демона
	*/
	virtual ~NetDaemon();
	
	/**
	* Вернуть число подконтрольных объектов
	*/
	int getObjectCount() const { return count; }
	
	/**
	* Вернуть максимальное число подконтрольных объектов
	*/
	int getObjectLimit() const { return limit; }
	
	/**
	* Добавить асинхронный объект
	*/
	bool addObject(ptr<NetObject> object);
	
	/**
	* Уведомить NetDaemon, что объект изменил свою маску
	*/
	void modifyObject(ptr<NetObject> object);
	
	/**
	* Удалить асинхронный объект
	*/
	bool removeObject(ptr<NetObject> object);
	
	/**
	* Запустить демона
	*/
	void run();
	
	/**
	* Завершить работу демона
	*/
	void terminate();
};

#endif // NANOSOFT_NETDAEMON_H
