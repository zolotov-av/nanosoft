#ifndef NANOSOFT_NETDAEMON_H
#define NANOSOFT_NETDAEMON_H

#include <sys/epoll.h>
#include <unistd.h>
#include <stdio.h>
#include <queue>
#include <nanosoft/object.h>
#include <nanosoft/asyncobject.h>
#include <nanosoft/mutex.h>

/**
* Callback таймера
*/
typedef void (*timer_callback_t) (int wid, void *data);

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
	
	struct timer
	{
		/**
		* Время когда надо активировать таймер
		*/
		int expires;
		
		/**
		* Callback таймера
		*/
		timer_callback_t callback;
		
		/**
		* Указатель на пользовательские данные
		*/
		void *data;
		
		/**
		* Конструктор по умолчанию
		*/
		timer() {
		}
		
		/**
		* Конструктор
		*/
		timer(int aExpires, timer_callback_t aCallback, void *aData):
			expires(aExpires), callback(aCallback), data(aData)
		{
		}
		
		/**
		* Оператор сравнения для приоритетной очереди
		*/
		bool operator < (const timer &t) const {
			return expires < t.expires;
		}
		
		/**
		* Запустить таймер
		*/
		void fire(int wid) {
			callback(wid, data);
		}
	};
	
	/**
	* Очередь таймеров
	*/
	typedef std::priority_queue<timer> timers_queue_t;
	
	/**
	* Таймеры
	*/
	timers_queue_t timers;
	
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
	nanosoft::ptr<AsyncObject> *objects;
	
	/**
	* Итератор объектов для корректного останова
	*/
	size_t iter;
	
	/**
	* Число потоков-воркеров
	*/
	int workerCount;
	
	/**
	* Число активных воркеров
	*/
	int activeCount;
	
	/**
	* Число таймеров в очереди
	*/
	int timerCount;
	
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
	bool resetObject(nanosoft::ptr<AsyncObject> &object);
	
	/**
	* Размер стека воркера
	*/
	size_t workerStackSize;
	
	/**
	* Вернуть время следущего таймера
	* @return время (Unix time) следующего таймера или -1 если таймеров нет
	*/
	int nextTimer();
	
	/**
	* Обработать таймеры
	*/
	void processTimers(int wid);
	
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
	* Вернуть число подконтрольных объектов
	*/
	int getObjectCount();
	
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
	bool addObject(nanosoft::ptr<AsyncObject> object);
	
	/**
	* Уведомить NetDaemon, что объект изменил свою маску
	*/
	void modifyObject(nanosoft::ptr<AsyncObject> object);
	
	/**
	* Удалить асинхронный объект
	*/
	bool removeObject(nanosoft::ptr<AsyncObject> object);
	
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
	* Установить таймер
	* @param expires время запуска таймера
	* @param callback функция обратного вызова
	* @param data указатель на пользовательские данные
	*/
	void setTimer(int expires, timer_callback_t callback, void *data);
	
	/**
	* Обработчик ошибок
	*
	* По умолчанию выводит все ошибки в stderr
	*/
	virtual void onError(const char *message);
};

#endif // NANOSOFT_NETDAEMON_H
