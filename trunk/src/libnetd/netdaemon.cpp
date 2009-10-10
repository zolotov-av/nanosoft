
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
NetDaemon::NetDaemon(int maxStreams)
{
	epoll = epoll_create(maxStreams);
}

/**
* Деструктор демона
*/
NetDaemon::~NetDaemon()
{
	close(epoll);
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
* Добавить поток
*/
bool NetDaemon::addStream(AsyncStream *stream)
{
	streams[stream->fd] = stream;
	struct epoll_event event;
	event.events = EPOLLIN | EPOLLONESHOT | EPOLLRDHUP;
	event.data.fd = stream->fd;
	return epoll_ctl(epoll, EPOLL_CTL_ADD, stream->fd, &event) == 0;
}

/**
* Возобновить работу с потоком
*/
bool NetDaemon::resetStream(AsyncStream *stream)
{
	struct epoll_event event;
	event.events = EPOLLIN | EPOLLONESHOT | EPOLLRDHUP;
	event.data.fd = stream->fd;
	return epoll_ctl(epoll, EPOLL_CTL_MOD, stream->fd, &event) == 0;
}

/**
* Ожидать поток
*/
AsyncStream* NetDaemon::waitStream()
{
	struct epoll_event event;
	while ( 1 )
	{
		int r = epoll_wait(epoll, &event, 1, -1);
		if ( r > 0 )
		{
			if ( event.events & EPOLLRDHUP )
			{
				streams[event.data.fd]->onShutdown();
				continue;
			}
			return streams[event.data.fd];
		}
		if ( r < 0 ) stderror();
	}
	return 0;
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
	Context *context = static_cast<Context *>(pContext);
	
	while ( 1 )
	{
		AsyncStream *s = context->d->waitStream();
		cerr << "#" << context->tid << ": ";
		if ( s == 0 ) break;
		else s->onRead();
		context->d->resetStream(s);
	}
	
	context->d->onError("worker exiting");
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
		cerr << "create #" << i << endl;
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
	startWorkers();
	Context *context = new Context;
	context->d = this;
	context->tid = -1;
	workerEntry(context);
	stopWorkers();
	return 0;
}
