#ifndef NANOSOFT_NETDAEMON_H
#define NANOSOFT_NETDAEMON_H

#include <sys/epoll.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <map>
#include <nanosoft/asyncobject.h>

/**
* Главный класс сетевого демона
*/
class NetDaemon
{
private:
	/**
	* Состояние демона
	*/
	enum state_t {
		/**
		* Работает - нормальное состояние
		*/
		running,
		
		/**
		* Завершение - демон в состоянии завершения работы
		*/
		terminating
	} state;
	
	/**
	* Файловый дескриптор epoll
	*/
	int epoll;
	
	/**
	* PID основного процесса
	*/
	pid_t master_pid;
	
	typedef std::map<int, AsyncObject*> map_objects_t;
	
	/**
	* Карта объектов (fd -> AsyncObject)
	*/
	map_objects_t objects;
	
	/**
	* Число потоков-воркеров
	*/
	int workerCount;
	
	/**
	* Список воркеров
	*/
	struct worker_info { pthread_t thread; pthread_attr_t attr; } *workers;
	
	/**
	* Признак активности демона
	* TRUE - демон работает
	* FALSE - демон остановлен или в процессе остановки
	*/
	bool active;
	
	/**
	* Точка входа в воркер
	*/
	static void * workerEntry(void *pContext);
	
	/**
	* Возобновить работу с асинхронным объектом
	*/
	bool resetObject(AsyncObject *object);
	
	/**
	* Размер стека воркера
	*/
	size_t workerStackSize;
	
protected:
	/**
	* Запустить воркеров
	*/
	void startWorkers();
	
	/**
	* Остановить воркеров
	*/
	void stopWorkers();
	
	/**
	* Ожидать завершения работы всех воркеров
	*/
	void waitWorkers();
	
	/**
	* Удалить воркеров
	*/
	void freeWorkers();
	
	/**
	* Послать сигнал onTerminate() всем подконтрольным объектам
	*/
	void killObjects();
	
	/**
	* Обработка системной ошибки
	*/
	void stderror();
public:
	/**
	* Конструктор демона
	* @param maxStreams максимальное число одновременных виртуальных потоков
	*/
	NetDaemon(int maxStreams);
	
	/**
	* Деструктор демона
	*/
	virtual ~NetDaemon();
	
	/**
	* Вернуть число воркеров
	*/
	int getWorkerCount();
	
	/**
	* Установить число воркеров
	*/
	void setWorkerCount(int count);
	
	/**
	* Вернуть размер стека воркера
	*/
	size_t getWorkerStackSize();
	
	/**
	* Установить размер стека воркера
	*/
	void setWorkerStackSize(size_t size);
	
	/**
	* Добавить асинхронный объект
	*/
	bool addObject(AsyncObject *object);
	
	/**
	* Удалить асинхронный объект
	*/
	bool removeObject(AsyncObject *object);
	
	/**
	* Запустить демона
	*/
	int run();
	
	/**
	* Завершить работу демона
	*/
	void terminate();
	
	/**
	* Обработчик ошибок
	*
	* По умолчанию выводит все ошибки в stderr
	*/
	virtual void onError(const char *message);
};

#endif // NANOSOFT_NETDAEMON_H
