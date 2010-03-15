
#include <sys/epoll.h>
#include <unistd.h>
#include <pthread.h>
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
NetDaemon::NetDaemon(int maxObjects):
	workerStackSize(DEFAULT_WORKER_STACK_SIZE),
	workerCount(0),
	activeCount(0),
	timerCount(0),
	count(0)
{
	epoll = epoll_create(maxObjects);
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
	workerCount = count;
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
bool NetDaemon::addObject(AsyncObject *object)
{
	fprintf(stderr, "#d: [NetDaemon] addObject(%d)\n", object->fd);
	struct epoll_event event;
	mutex.lock();
		objects[object->fd] = object;
		count ++;
		//fprintf(stderr, "AddObject(%d), count = %d\n", object->fd, count);
		event.events = object->getEventsMask();
		event.data.fd = object->fd;
		bool r = epoll_ctl(epoll, EPOLL_CTL_ADD, object->fd, &event) == 0;
	mutex.unlock();
	//fprintf(stderr, "addObject leave, count: %d\n", objects.size());
	return r;
}

/**
* Уведомить NetDaemon, что объект изменил свою маску
*/
void NetDaemon::modifyObject(AsyncObject *object)
{
	//fprintf(stderr, "#d: [NetDaemon] modifyObject(%d)\n", object->fd);
	mutex.lock();
		map_objects_t::iterator pos = objects.find(object->fd);
		if ( pos != objects.end() )
		{
			resetObject(object);
		}
	mutex.unlock();
}

/**
* Удалить асинхронный объект
*/
bool NetDaemon::removeObject(AsyncObject *object)
{
	fprintf(stderr, "#d: [NetDaemon] removeObject(%d)\n", object->fd);
	mutex.lock();
	//fprintf(stderr, "#%d NetDaemon::removeObject(%d) enter, count = %d\n", object->workerId, object->fd, count);
	map_objects_t::iterator pos = objects.find(object->fd);
	if ( pos != objects.end() )
	{
		objects.erase(object->fd);
		if ( epoll_ctl(epoll, EPOLL_CTL_DEL, object->fd, 0) != 0 ) stderror();
		count --;
		if ( count == 0 ) stopWorkers();
	}
	else
	{
		fprintf(stderr, "#%d NetDaemon::removeObject(%d): not found o.O\n", object->workerId, object->fd);
	}
	//fprintf(stderr, "#%d removeObject(%d) leave, count = %d\n", object->workerId, object->fd, count);
	mutex.unlock();
}

/**
* Возобновить работу с асинхронным объектом
*/
bool NetDaemon::resetObject(AsyncObject *object)
{
	//fprintf(stderr, "#%d: NetDaemon::resetObject(%d)\n", object->workerId, object->fd);
	struct epoll_event event;
	event.events = object->getEventsMask();
	event.data.fd = object->fd;
	if ( epoll_ctl(epoll, EPOLL_CTL_MOD, object->fd, &event) != 0 )
	{
		stderror();
	}
}

/**
* Действие активного цикла
*/
void NetDaemon::doActiveAction(worker_t *worker)
{
	AsyncObject *obj;
	struct epoll_event event;
	int r = epoll_wait(epoll, &event, 1, nextTimer());
	if ( r > 0 )
	{
		mutex.lock();
			map_objects_t::iterator pos = objects.find(event.data.fd);
			obj = (pos != objects.end()) ? pos->second : 0;
		mutex.unlock();
		if ( obj )
		{
			obj->workerId = worker->workerId;
			obj->onEvent(event.events);
			mutex.lock();
				if ( objects.find(event.data.fd) != objects.end() )
				{
					resetObject(obj);
				}
			mutex.unlock();
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
	if ( ! worker->checked ) {
		worker->checked = true;
		mutex.lock();
			activeCount --;
			if ( activeCount == 0 )
			{
				iter = objects.begin();
				setWorkersState(TERMINATE);
			}
		mutex.unlock();
		return;
	}
	
	fprintf(stderr, "#%d: doSleepAction\n", worker->workerId);
	
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
	fprintf(stderr, "#%d: doTerminateAction\n", worker->workerId);
	
	AsyncObject *obj;
	mutex.lock();
		if ( iter != objects.end() )
		{
			obj = iter->second;
			++ iter;
		}
		else
		{
			setWorkersState(ACTIVE);
			obj = 0;
		}
	mutex.unlock();
	if ( obj )
	{
		obj->workerId = worker->workerId;
		obj->onTerminate();
	}
}

/**
* Точка входа в воркер
*/
void* NetDaemon::workerEntry(void *pWorker)
{
	worker_t *worker = static_cast<worker_t *>(pWorker);
	NetDaemon *daemon = worker->daemon;
	
	fprintf(stderr, "#%d: worker started\n", worker->workerId);
	
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
	
	fprintf(stderr, "#%d: worker exited\n", worker->workerId);
	
	return 0;
}

/**
* Запустить воркеров
*/
void NetDaemon::startWorkers()
{
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
}

/**
* Сменить статус воркеров
*/
void NetDaemon::setWorkersState(worker_status_t status)
{
	for(int i = 0; i < workerCount; i++)
	{
		workers[i].status = status;
		workers[i].checked = false;
		pthread_kill(workers[i].thread, SIGHUP);
	}
	main.status = status;
	main.checked = false;
	pthread_kill(main.thread, SIGHUP);
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
	for(int i = 0; i < workerCount; i++)
	{
		void *status;
		int rc = pthread_join(workers[i].thread, &status);
		if ( rc ) fprintf(stderr, "[NetDaemon] pthread_join(#%) fault\n", i+1);
	}
}

/**
* Удалить воркеров
*/
void NetDaemon::freeWorkers()
{
	for(int i = 0; i < workerCount; i++)
	{
		pthread_attr_destroy(&workers[i].attr);
	}
	delete [] workers;
}

/**
* Послать сигнал onTerminate() всем подконтрольным объектам
*/
void NetDaemon::killObjects()
{
	// TODO добавить mutex
	for(map_objects_t::iterator pos = objects.begin(); pos != objects.end(); ++pos)
	{
		pos->second->terminate();
	}
}

/**
* Запустить демона
*/
int NetDaemon::run()
{
	startWorkers();
	main.daemon = this;
	main.workerId = 0;
	main.thread = pthread_self();
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
	pthread_t tid = pthread_self();
	for(int i = 0; i < workerCount; i++)
	{
		if ( pthread_equal(workers[i].thread, tid) ) return i + 1;
	}
	if ( pthread_equal(main.thread, tid) ) return 0;
	return -1;
}

/**
* Завершить работу демона
*/
void NetDaemon::terminate()
{
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
	bool hup = false;
	mutex.lock();
		timers.push(timer(expires, callback, data));
		hup = timerCount = 0;
		timerCount++;
	mutex.unlock();
	if ( hup )
	{
		for(int i = 0; i < workerCount; i++)
		{
			pthread_kill(workers[i].thread, SIGHUP);
		}
		pthread_kill(main.thread, SIGHUP);
	}
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
}
