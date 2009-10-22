
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
	count(0)
{
	epoll = epoll_create(maxObjects);
	stdcheck( pthread_mutex_init(&mutex, 0) == 0 );
}

/**
* Деструктор демона
*/
NetDaemon::~NetDaemon()
{
	int r = ::close(epoll);
	if ( r < 0 ) stderror();
	stdcheck( pthread_mutex_destroy(&mutex) == 0 );
}

/**
* Получить монопольный доступ к NetDaemon
*/
void NetDaemon::lock() {
	stdcheck( pthread_mutex_lock(&mutex) == 0 );
}

/**
* Освободить NetDaemon
*/
void NetDaemon::unlock() {
	stdcheck( pthread_mutex_unlock(&mutex) == 0 );
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
	//fprintf(stderr, "addObject enter, count: %d\n", objects.size());
	struct epoll_event event;
	lock();
		objects[object->fd] = object;
		count ++;
		fprintf(stderr, "AddObject(%d), count = %d\n", object->fd, count);
		event.events = object->getEventsMask();
		event.data.fd = object->fd;
		bool r = epoll_ctl(epoll, EPOLL_CTL_ADD, object->fd, &event) == 0;
	unlock();
	//fprintf(stderr, "addObject leave, count: %d\n", objects.size());
	return r;
}

/**
* Удалить асинхронный объект
*/
bool NetDaemon::removeObject(AsyncObject *object)
{
	//fprintf(stderr, "#%d: NetDaemon::removeObject\n", object->workerId);
	lock();
	fprintf(stderr, "removeObject(%d) enter, count = %d\n", object->fd, count);
	map_objects_t::iterator pos = objects.find(object->fd);
	if ( pos != objects.end() )
	{
		fprintf(stderr, "objects.erase(%d) = %d\n", object->fd, objects.erase(object->fd));
		if ( epoll_ctl(epoll, EPOLL_CTL_DEL, object->fd, 0) != 0 ) stderror();
		count --;
		if ( count == 0 ) stopWorkers();
	}
	else
	{
		fprintf(stderr, "objects.find(%d) == end() o.O\n");
	}
	fprintf(stderr, "removeObject(%d) leave, count = %d\n", object->fd, count);
	unlock();
}

/**
* Возобновить работу с асинхронным объектом
*/
bool NetDaemon::resetObject(AsyncObject *object)
{
	fprintf(stderr, "#%d: NetDaemon::resetObject(%d)\n", object->workerId, object->fd);
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
	struct epoll_event event;
	int r = epoll_wait(epoll, &event, 1, -1);
	if ( r > 0 )
	{
		lock();
			AsyncObject *obj = objects[event.data.fd];
		unlock();
		obj->workerId = worker->workerId;
		obj->onEvent(event.events);
		lock();
		if ( objects[event.data.fd] != 0 )
		{
			fprintf(stderr, "#%d: resetObject\n", worker->workerId);
			resetObject(obj);
		}
		unlock();
	}
	if ( r < 0 ) fprintf(stderr, "#%d: %s\n", worker->workerId, nanosoft::stderror());
	if ( r == 0 ) fprintf(stderr, "#%d: skip\n", worker->workerId);
}

/**
* Действие спящего цикла
*/
void NetDaemon::doSleepAction(worker_t *worker)
{
	if ( ! worker->checked ) {
		worker->checked = true;
		lock();
			activeCount --;
			if ( activeCount == 0 )
			{
				iter = objects.begin();
				setWorkersState(TERMINATE);
			}
		unlock();
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
		lock();
			activeCount ++;
		unlock();
		return;
	}
	fprintf(stderr, "#%d: doTerminateAction\n", worker->workerId);
	
	AsyncObject *obj;
	lock();
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
	unlock();
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
		lock();
		activeCount++;
		unlock();
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
		pos->second->onTerminate();
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
	lock();
		activeCount++;
	unlock();
	workerEntry(&main);
	waitWorkers();
	freeWorkers();
	return 0;
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
