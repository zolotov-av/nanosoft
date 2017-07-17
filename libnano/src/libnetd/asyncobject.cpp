#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <nanosoft/asyncobject.h>
#include <nanosoft/netdaemon.h>

using namespace std;

/**
* Конструктор
*/
AsyncObject::AsyncObject(): fd(0), terminating(false)
{
}

/**
* Конструктор
*/
AsyncObject::AsyncObject(int afd): fd(afd), terminating(false)
{
}

/**
* Деструктор
*/
AsyncObject::~AsyncObject()
{
}

/**
* Обработка системной ошибки
*/
void AsyncObject::stderror()
{
	onError(strerror(errno));
}

/**
* Установить файловый дескриптор
*/
void AsyncObject::setFd(int v)
{
	fd = v;
}

/**
* Событие ошибки
*
* Вызывается в случае возникновения какой-либо ошибки
*/
void AsyncObject::onError(const char *message)
{
	fprintf(stderr, "AsyncObject[%d]: %s\n", fd, message);
}

/**
* Послать сигнал завершения
*/
void AsyncObject::terminate()
{
	fprintf(stderr, "AsyncObject[%d]: terminate(%d)...\n", fd, terminating);
	if ( ! terminating )
	{
		terminating = true;
		onTerminate();
	}
}

/**
* Отсоединиться от сервера
*/
void AsyncObject::dettach()
{
	if ( daemon )
	{
		daemon->removeObject(this);
	}
}
