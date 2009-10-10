
#include <sys/epoll.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <nanosoft/netdaemon.h>
#include <iostream>
#include <errno.h>
#include <string.h>

using namespace std;

/**
* Конструктор демона
* @param maxStreams максимальное число одновременных виртуальных потоков
*/
NetDaemon::NetDaemon(int maxObjects)
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
}

/**
* Возобновить работу с асинхронным объектом
*/
bool NetDaemon::resetObject(AsyncObject *object)
{cerr << "resetObj" << endl;
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
	{cerr << "wait in #" << context->tid << endl;
		int r = epoll_wait(daemon->epoll, &event, 1, -1);
		if ( r > 0 )
		{
			AsyncObject *obj = daemon->objects[event.data.fd];
			obj->onEvent(event.events);
			daemon->resetObject(obj);
		}
		if ( r < 0 ) daemon->stderror();
		if ( r == 0 ) daemon->onError("skip");
	}
	
	daemon->onError("worker exiting");
	delete context;
	return 0;
}

/**
* Запустить воркеров
*/
void NetDaemon::startWorkers()
{
	//printf("todo startWorkers\n");
	workers = new pthread_t[workerCount];
	for(int i = 0; i < workerCount; i++)
	{
		Context *context = new Context;
		context->d = this;
		context->tid = i;
		pthread_create(&workers[i], NULL, workerEntry, context);
	}
}

/**
* Остановить воркеров
*/
void NetDaemon::stopWorkers()
{
	//printf("todo stopWorkers\n");
	delete [] workers;
}

/**
* Запустить демона
*/
int NetDaemon::run()
{
	active = true;
	startWorkers();
	Context *context = new Context;
	context->d = this;
	context->tid = -1;
	workerEntry(context);
	stopWorkers();
	return 0;
}

/**
* Завершить работу демона
*/
void NetDaemon::terminate(int code)
{
	onError("terminate...");
	exitCode = code;
	active = false;
}
