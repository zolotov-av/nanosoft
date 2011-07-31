
#include <nanosoft/core/netdaemon.h>

#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

/**
* Конструктор демона
* @param maxStreams максимальное число одновременных виртуальных потоков
*/
NetDaemon::NetDaemon(int aLimit):
	count(0), state(INACTIVE), timer(0)
{
	limit = aLimit;
	objects = new ptr<NetObject>[aLimit];
	epoll = epoll_create(aLimit);
	for(int i = 0; i < aLimit; i++) objects[i] = 0;
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
* Обработка системной ошибки
*/
void NetDaemon::stderror()
{
	fprintf(stderr, "NetDaemon: %s\n", strerror(errno));
}

/**
* Добавить асинхронный объект
*/
bool NetDaemon::addObject(ptr<NetObject> object)
{
	int r = false;
	struct epoll_event event;
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
		if ( ! r  )
		{
			stderror();
			count --;
			objects[object->fd] = 0;
		}
	}
	return r;
}

/**
* Уведомить NetDaemon, что объект изменил свою маску
*/
void NetDaemon::modifyObject(ptr<NetObject> object)
{
	if ( objects[object->fd] == object )
	{
		resetObject(object);
	}
}

/**
* Удалить асинхронный объект
*/
bool NetDaemon::removeObject(ptr<NetObject> object)
{
	if ( objects[object->fd] == object )
	{
		object->release();
		if ( epoll_ctl(epoll, EPOLL_CTL_DEL, object->fd, 0) != 0 ) stderror();
		objects[object->fd] = 0;
		count --;
		if ( count == 0 ) state = INACTIVE;
	}
}

/**
* Возобновить работу с асинхронным объектом
*/
bool NetDaemon::resetObject(ptr<NetObject> &object)
{
	struct epoll_event event;
	event.events = object->getEventsMask();
	event.data.fd = object->fd;
	if ( epoll_ctl(epoll, EPOLL_CTL_MOD, object->fd, &event) != 0 ) stderror();
}

/**
* Запустить демона
*/
void NetDaemon::run()
{
	state = ACTIVE;
	struct epoll_event event;
	tm = time(0);
	while ( state == ACTIVE )
	{
		int r = epoll_wait(epoll, &event, 1, 200);
		if ( r > 0 )
		{
			ptr<NetObject> obj = objects[event.data.fd];
			if ( obj != 0 )
			{
				obj->onEvent(event.events);
				if ( objects[event.data.fd] == obj )
				{
					resetObject(obj);
				}
			}
		}
		else if ( r < 0 ) stderror();
		
		time_t ntm = time(0);
		if ( tm != ntm )
		{
			tm = ntm;
			if ( timer ) timer();
		}
	}
	state = INACTIVE;
}

/**
* Завершить работу демона
*/
void NetDaemon::terminate()
{
	// TODO
}
