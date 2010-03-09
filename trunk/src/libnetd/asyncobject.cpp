#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <nanosoft/asyncobject.h>

using namespace std;

/**
* Конструктор
*/
AsyncObject::AsyncObject(): fd(0), workerId(-1), terminating(false)
{
}

/**
* Конструктор
*/
AsyncObject::AsyncObject(int afd): fd(afd), workerId(-1), terminating(false)
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
* Событие ошибки
*
* Вызывается в случае возникновения какой-либо ошибки
*/
void AsyncObject::onError(const char *message)
{
	fprintf(stderr, "#%d: [AsyncObject: %d]: %s\n", getWorkerId(), fd, message);
}

/**
* Послать сигнал завершения
*/
void AsyncObject::terminate()
{
	fprintf(stderr, "#%d: [AsyncObject: %d]: terminate(%d)...\n", getWorkerId(), fd, terminating);
	if ( ! terminating )
	{
		terminating = true;
		onTerminate();
	}
}
