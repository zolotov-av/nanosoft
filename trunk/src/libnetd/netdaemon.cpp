
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <nanosoft/netdaemon.h>
#include <nanosoft/error.h>
#include <iostream>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

using namespace std;
using namespace nanosoft;

#define DEFAULT_WORKER_STACK_SIZE (sizeof(size_t) * 1024 * 1024)

/**
* Конструктор демона
* @param maxStreams максимальное число одновременных виртуальных потоков
*/
NetDaemon::NetDaemon(int maxStreams):
	workerStackSize(DEFAULT_WORKER_STACK_SIZE),
	workerCount(0),
	activeCount(0),
	timerCount(0),
	count(0),
	active(0)
{
	limit = maxStreams;
	objects = new nanosoft::ptr<AsyncObject>[limit];
	epoll = epoll_create(maxStreams);
	for(int i = 0; i < limit; i++) objects[i] = 0;
}

/**
* Деструктор демона
*/
NetDaemon::~NetDaemon()
{
	int r = ::close(epoll);
	if ( r < 0 ) stderror();
}

/**
* Обработчик ошибок
*
* По умолчанию выводит все ошибки в stderr
*/
void NetDaemon::onError(const char *message)
{
	cerr << "[NetDaemon]: " << message << endl;
}

/**
* Обработка системной ошибки
*/
void NetDaemon::stderror()
{
	onError(strerror(errno));
}

/**
* Вернуть число воркеров
*/
int NetDaemon::getWorkerCount()
{
	return workerCount;
}

/**
* Установить число воркеров
*/
void NetDaemon::setWorkerCount(int count)
{
#ifdef USE_PTHREAD
	workerCount = count;
#endif
}

/**
* Вернуть число подконтрольных объектов
*/
int NetDaemon::getObjectCount()
{
	return count;
}

/**
* Вернуть размер стека воркера
*/
size_t NetDaemon::getWorkerStackSize()
{
	return workerStackSize;
}

/**
* Установить размер стека воркера
*/
void NetDaemon::setWorkerStackSize(size_t size)
{
	workerStackSize = size;
}

/**
* Добавить асинхронный объект
*/
bool NetDaemon::addObject(nanosoft::ptr<AsyncObject> object)
{
	int r = false;
	struct epoll_event event;
	mutex.lock();
		if ( objects[object->fd] == 0 )
		{
			count ++;
			objects[object->fd] = object;
			object->lock();
			
			// принудительно выставить O_NONBLOCK
			int flags = fcntl(object->fd, F_GETFL, 0);
			if ( flags >= 0 )
			{
				fcntl(object->fd, F_SETFL, flags | O_NONBLOCK);
			}
			
			event.events = object->getEventsMask();
			event.data.fd = object->fd;
			r = epoll_ctl(epoll, EPOLL_CTL_ADD, object->fd, &event) == 0;
			/*if ( ! r  )
			{
				stderror();
				count --;
				objects[object->fd] = 0;
			}*/
		}
	mutex.unlock();
	return r;
}

/**
* Уведомить NetDaemon, что объект изменил свою маску
*/
void NetDaemon::modifyObject(nanosoft::ptr<AsyncObject> object)
{
	mutex.lock();
		if ( objects[object->fd] == object )
		{
			resetObject(object);
		}
	mutex.unlock();
}

/**
* Удалить асинхронный объект
*/
bool NetDaemon::removeObject(nanosoft::ptr<AsyncObject> object)
{
	mutex.lock();
		if ( objects[object->fd] == object )
		{
			object->release();
			if ( epoll_ctl(epoll, EPOLL_CTL_DEL, object->fd, 0) != 0 ) stderror();
			objects[object->fd] = 0;
			count --;
			if ( count == 0 ) stopWorkers();
		}
	mutex.unlock();
}

/**
* Возобновить работу с асинхронным объектом
*/
bool NetDaemon::resetObject(nanosoft::ptr<AsyncObject> &object)
{
	struct epoll_event event;
	event.events = object->getEventsMask();
	event.data.fd = object->fd;
	if ( epoll_ctl(epoll, EPOLL_CTL_MOD, object->fd, &event) != 0 ) stderror();
}

/**
* Действие активного цикла
*/
void NetDaemon::doActiveAction(worker_t *worker)
{
	ptr<AsyncObject> obj;
	struct epoll_event event;
	int r = epoll_wait(epoll, &event, 1, nextTimer());
	if ( r > 0 )
	{
		mutex.lock();
			obj = objects[event.data.fd];
		mutex.unlock();
		if ( obj != 0 )
		{
			obj->workerId = worker->workerId;
			obj->onEvent(event.events);
			mutex.lock();
				if ( objects[event.data.fd] == obj )
				{
					resetObject(obj);
				}
			mutex.unlock();
			obj = 0;
		}
	}
	if ( r < 0 ) fprintf(stderr, "#%d: %s\n", worker->workerId, nanosoft::stderror());
	if ( r == 0 ) processTimers(worker->workerId);
}

/**
* Действие спящего цикла
*/
void NetDaemon::doSleepAction(worker_t *worker)
{
	if ( ! worker->checked )
	{
		worker->checked = true;
		mutex.lock();
			activeCount --;
			if ( activeCount == 0 )
			{
				iter = 0;
				setWorkersState(TERMINATE);
			}
		mutex.unlock();
		return;
	}
	
	printf("#%d: doSleepAction\n", worker->workerId);
	
	// просто засыпаем на как можно больший срок
	// лишь сигнал нас должен разбудить
	struct timespec tm;
	tm.tv_sec = 999999999;
	tm.tv_nsec = 0;
	nanosleep(&tm, 0);
}

/**
* Действие завещающего цикла
*/
void NetDaemon::doTerminateAction(worker_t *worker)
{
	if ( ! worker->checked )
	{
		worker->checked = true;
		mutex.lock();
			activeCount ++;
		mutex.unlock();
		return;
	}
	//printf("#%d: doTerminateAction\n", worker->workerId);
	
	ptr<AsyncObject> obj;
	mutex.lock();
		if ( iter < limit )
		{
			obj = objects[iter];
			++ iter;
		}
		else
		{
			setWorkersState(ACTIVE);
			obj = 0;
		}
	mutex.unlock();
	if ( obj != 0 )
	{
		obj->workerId = worker->workerId;
		obj->terminate();
	}
}

/**
* Точка входа в воркер
*/
void* NetDaemon::workerEntry(void *pWorker)
{
	worker_t *worker = static_cast<worker_t *>(pWorker);
	NetDaemon *daemon = worker->daemon;
	
	printf("#%d: worker started\n", worker->workerId);
	
	while ( daemon->count > 0 && worker->status != INACTIVE )
	{
		switch ( worker->status )
		{
		case ACTIVE:
			daemon->doActiveAction(worker);
			break;
		case SLEEP:
			daemon->doSleepAction(worker);
			break;
		case TERMINATE:
			daemon->doTerminateAction(worker);
			break;
		}
	}
	
	printf("#%d: worker exited\n", worker->workerId);
	
	return 0;
}

/**
* Запустить воркеров
*/
void NetDaemon::startWorkers()
{
#ifdef USE_PTHREAD
	workers = new worker_t[workerCount];
	for(int i = 0; i < workerCount; i++)
	{
		workers[i].daemon = this;
		workers[i].workerId = i + 1;
		workers[i].status = ACTIVE;
		mutex.lock();
			activeCount++;
		mutex.unlock();
		pthread_attr_init(&workers[i].attr);
		pthread_attr_setstacksize(&workers[i].attr, getWorkerStackSize());
		pthread_attr_setdetachstate(&workers[i].attr, PTHREAD_CREATE_JOINABLE);
		pthread_create(&workers[i].thread, &workers[i].attr, workerEntry, &workers[i]);
	}
#endif
}

/**
* Сменить статус воркеров
*/
void NetDaemon::setWorkersState(worker_status_t status)
{
#ifdef USE_PTHREAD
	for(int i = 0; i < workerCount; i++)
	{
		workers[i].status = status;
		workers[i].checked = false;
		pthread_kill(workers[i].thread, SIGHUP);
	}
#endif
	main.status = status;
	main.checked = false;
#ifdef USE_PTHREAD
	pthread_kill(main.thread, SIGHUP);
#endif
}

/**
* Остановить воркеров
*/
void NetDaemon::stopWorkers()
{
	cerr << "NetDaemon::stopWorkers()...\n";
	setWorkersState(INACTIVE);
}

/**
* Ожидать завершения работы всех воркеров
*/
void NetDaemon::waitWorkers()
{
#ifdef USE_PTHREAD
	for(int i = 0; i < workerCount; i++)
	{
		void *status;
		int rc = pthread_join(workers[i].thread, &status);
		if ( rc ) fprintf(stderr, "[NetDaemon] pthread_join(#%) fault\n", i+1);
	}
#endif
}

/**
* Удалить воркеров
*/
void NetDaemon::freeWorkers()
{
#ifdef USE_PTHREAD
	for(int i = 0; i < workerCount; i++)
	{
		pthread_attr_destroy(&workers[i].attr);
	}
	delete [] workers;
#endif
}

/**
* Запустить демона
*/
int NetDaemon::run()
{
	active = 1;
	startWorkers();
	main.daemon = this;
	main.workerId = 0;
#ifdef USE_PTHREAD
	main.thread = pthread_self();
#endif
	main.status = ACTIVE;
	mutex.lock();
		activeCount++;
	mutex.unlock();
	workerEntry(&main);
	waitWorkers();
	freeWorkers();
	return 0;
}

/**
* Вернуть ID текущего воркера
*/
int NetDaemon::wid()
{
#ifdef USE_PTHREAD
	pthread_t tid = pthread_self();
	for(int i = 0; i < workerCount; i++)
	{
		if ( pthread_equal(workers[i].thread, tid) ) return i + 1;
	}
	if ( pthread_equal(main.thread, tid) ) return 0;
	return -1;
#endif
	return 0;
}

/**
* Завершить работу демона
*/
void NetDaemon::terminate()
{
	active = 0;
	onError("terminate...");
	//killObjects();
	setWorkersState(SLEEP);
}

/**
* Установить таймер
* @param expires время запуска таймера
* @param callback функция обратного вызова
* @param data указатель на пользовательские данные
*/
void NetDaemon::setTimer(int expires, timer_callback_t callback, void *data)
{
	printf("setTimer(%d)\n", expires);
	bool hup = false;
	mutex.lock();
		timers.push(timer(expires, callback, data));
		hup = timerCount = 0;
		timerCount++;
	mutex.unlock();
#ifdef USE_PTHREAD
	if ( hup )
	{
		for(int i = 0; i < workerCount; i++)
		{
			pthread_kill(workers[i].thread, SIGHUP);
		}
		pthread_kill(main.thread, SIGHUP);
	}
#endif
}

/**
* Вернуть время следущего таймера
* @return время (Unix time) следующего таймера или -1 если таймеров нет
*/
int NetDaemon::nextTimer()
{
	int timeout = -1;
	int expires;
	if ( timerCount > 0 ) {
		mutex.lock();
			expires = timers.top().expires - time(0);
		mutex.unlock();
		timeout = expires > 0 ? expires * 1000 : 0;
	}
	return timeout;
}

/**
* Обработать таймеры
*/
void NetDaemon::processTimers(int wid)
{
	printf("processTimers(%d) enter\n", wid);
	timer t;
	time_t now = time(0);
	while ( timerCount > 0 )
	{
		bool process = false;
		mutex.lock();
			t = timers.top();
			if ( t.expires <= now )
			{
				timers.pop();
				timerCount --;
				process = true;
			}
		mutex.unlock();
		if ( process ) t.fire(wid);
		else break;
	}
	printf("processTimers(%d) leave\n", wid);
}
