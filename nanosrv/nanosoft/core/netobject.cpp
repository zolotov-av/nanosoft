#include <nanosoft/core/netobject.h>

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

/**
* Конструктор
*/
NetObject::NetObject(): fd(0), terminating(false)
{
}

/**
* Конструктор
*/
NetObject::NetObject(int afd): fd(afd), terminating(false)
{
}

/**
* Деструктор
*/
NetObject::~NetObject()
{
}

/**
* Обработка системной ошибки
*/
void NetObject::stderror()
{
	fprintf(stderr, "NetObject: %s\n", strerror(errno));
}

/**
* Неблокирующее чтение из потока
*/
ssize_t NetObject::read(void *buf, size_t count)
{
	ssize_t r = ::read(fd, buf, count);
	if ( r < 0 ) stderror();
	return r;
}

/**
* Неблокирующая запись в поток
*/
ssize_t NetObject::write(const void *buf, size_t count)
{
	ssize_t r = ::write(fd, buf, count);
	if ( r < 0 ) stderror();
	return r;
}

/**
* Послать сигнал завершения
*/
void NetObject::terminate()
{
	if ( ! terminating )
	{
		terminating = true;
		onTerminate();
	}
}
