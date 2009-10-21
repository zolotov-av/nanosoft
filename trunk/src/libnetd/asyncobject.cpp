#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <nanosoft/asyncobject.h>

using namespace std;

/**
* Конструктор
*/
AsyncObject::AsyncObject(): fd(0), workerId(-1)
{
}

/**
* Конструктор
*/
AsyncObject::AsyncObject(int afd): fd(afd)
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
	cerr << "[AsyncObject]: " << message << endl;
}
