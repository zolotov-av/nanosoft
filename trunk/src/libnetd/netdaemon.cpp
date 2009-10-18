
#include <sys/epoll.h>
#include <unistd.h>
#include <pthread.h>
#include <nanosoft/netdaemon.h>
#include <iostream>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

using namespace std;

#define DEFAULT_WORKER_STACK_SIZE (sizeof(size_t) * 1024 * 1024)

/**
* Конструктор демона
* @param maxStreams максимальное число одновременных виртуальных потоков
*/
NetDaemon::NetDaemon(int maxObjects): workerStackSize(DEFAULT_WORKER_STACK_SIZE)
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
	struct epoll_event event;
	objects[object->fd] = object;
	event.events = object->getEventsMask();
	event.data.fd = object->fd;
	return epoll_ctl(epoll, EPOLL_CTL_ADD, object->fd, &event) == 0;
}

/**
* Удалить асинхронный объект
*/
bool NetDaemon::removeObject(AsyncObject *object)
{
	if ( objects.erase(object->fd) > 0 )
	{
		if ( epoll_ctl(epoll, EPOLL_CTL_DEL, object->fd, 0) != 0 ) stderror();
	}
	if ( objects.empty() ) stopWorkers();
}

/**
* Возобновить работу с асинхронным объектом
*/
bool NetDaemon::resetObject(AsyncObject *object)
{
	struct epoll_event event;
	event.events = object->getEventsMask();
	event.data.fd = object->fd;
	if ( epoll_ctl(epoll, EPOLL_CTL_MOD, object->fd, &event) != 0 )
	{
		stderror();
	}
}

struct Context
{
	NetDaemon *d;
	int tid;
};

/**
* Точка входа в воркер
*/
void* NetDaemon::workerEntry(void *pContext)
{
	struct epoll_event event;
	Context *context = static_cast<Context *>(pContext);
	NetDaemon *daemon = context->d;
	
	while ( daemon->active )
	{
		int r = epoll_wait(daemon->epoll, &event, 1, -1);
		if ( r > 0 )
		{
			AsyncObject *obj = daemon->objects[event.data.fd];
			obj->onEvent(event.events);
			if ( daemon->objects[event.data.fd] != 0 )
			{
				daemon->resetObject(obj);
			}
		}
		if ( r < 0 ) daemon->stderror();
		if ( r == 0 ) daemon->onError("skip");
	}
	
	fprintf(stderr, "worker exiting #%d\n", context->tid);
	delete context;
	return 0;
}

/**
* Запустить воркеров
*/
void NetDaemon::startWorkers()
{
	workers = new worker_info[workerCount];
	for(int i = 0; i < workerCount; i++)
	{
		Context *context = new Context;
		context->d = this;
		context->tid = i + 1;
		pthread_attr_init(&workers[i].attr);
		pthread_attr_setstacksize(&workers[i].attr, getWorkerStackSize());
		pthread_attr_setdetachstate(&workers[i].attr, PTHREAD_CREATE_JOINABLE);
		pthread_create(&workers[i].thread, &workers[i].attr, workerEntry, context);
	}
}

/**
* Остановить воркеров
*/
void NetDaemon::stopWorkers()
{
	cerr << "NetDaemon::stopWorkers()...\n";
	active = false;
	for(int i = 0; i < workerCount; i++)
	{
		pthread_kill(workers[i].thread, SIGHUP);
	}
	kill(master_pid, SIGHUP);
}

/**
* Ожидать завершения работы всех воркеров
*/
void NetDaemon::waitWorkers()
{
	cerr << "[NetDaemon] wait for workers exited..." << endl;
	for(int i = 0; i < workerCount; i++)
	{
		void *status;
		int rc = pthread_join(workers[i].thread, &status);
		if ( rc ) cerr << "some error with thread #" << (i+1) << endl;
	}
	cerr << "[NetDaemon] workers exited." << endl;
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
	master_pid = getpid();
	active = true;
	startWorkers();
	Context *context = new Context;
	context->d = this;
	context->tid = 0;
	workerEntry(context);
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
	state = terminating;
	killObjects();
}
