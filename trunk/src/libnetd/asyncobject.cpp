#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <nanosoft/asyncobject.h>

using namespace std;

/**
* Обработка системной ошибки
*/
void AsyncObject::stderror()
{
	onError(strerror(errno));
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
* Событие ошибки
*
* Вызывается в случае возникновения какой-либо ошибки
*/
void AsyncObject::onError(const char *message)
{
	cerr << "[AsyncObject]: " << message << endl;
}
