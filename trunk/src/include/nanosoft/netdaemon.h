#ifndef NANOSOFT_NETDAEMON_H
#define NANOSOFT_NETDAEMON_H

#include <sys/epoll.h>
#include <unistd.h>
#include <stdio.h>
#include <map>
#include <nanosoft/asyncobject.h>
#include <nanosoft/mutex.h>

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
	* mutex
	*/
	nanosoft::Mutex mutex;
	
	typedef std::map<int, AsyncObject*> map_objects_t;
	
	/**
	* Карта объектов (fd -> AsyncObject)
	*/
	map_objects_t objects;
	
	/**
	* Число объектов в epoll
	*/
	int count;
	
	/**
	* Итератор объектов для корректного останова
	*/
	map_objects_t::iterator iter;
	
	/**
	* Число потоков-воркеров
	*/
	int workerCount;
	
	/**
	* Число активных воркеров
	*/
	int activeCount;
	
	/**
	* Статус воркера
	*/
	enum worker_status_t {
		/**
		* Поток не запущен
		*/
		INACTIVE,
		
		/**
		* Поток в обычном рабочем цикле
		*/
		ACTIVE,
		
		/**
		* Подготовка к останову - уснуть и ждать останова других потоков
		*/
		SLEEP,
		
		/**
		* Останов - послать всем объектам событие onTerminate()
		*/
		TERMINATE
	};
	
	/**
	* Информация о воркере
	*/
	struct worker_t {
		/**
		* Ссылка на демона
		*/
		NetDaemon *daemon;
		
		/**
		* ID воркера
		*/
		int workerId;
		
		/**
		* ID потока
		*/
		pthread_t thread;
		
		/**
		* Атрибуты потока
		*/
		pthread_attr_t attr;
		
		/**
		* Текущий статус воркера
		*/
		worker_status_t status;
		
		bool checked;
	};
	
	/**
	* Главный поток
	*/
	worker_t main;
	
	/**
	* Список воркеров
	*/
	worker_t *workers;
	
	/**
	* Действие активного цикла
	*/
	void doActiveAction(worker_t *worker);
	
	/**
	* Действие спящего цикла
	*/
	void doSleepAction(worker_t *worker);
	
	/**
	* Действие завещающего цикла
	*/
	void doTerminateAction(worker_t *worker);
	
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
	* Сменить статус воркеров
	*/
	void setWorkersState(worker_status_t status);
	
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
	* Вернуть ID текущего воркера
	*/
	int wid();
	
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
