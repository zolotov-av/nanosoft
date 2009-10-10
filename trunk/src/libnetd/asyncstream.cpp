#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <nanosoft/asyncstream.h>

using namespace std;

/**
* Конструктор
*/
AsyncStream::AsyncStream(int afd): fd(afd)
{
}

/**
* Деструктор
*/
AsyncStream::~AsyncStream()
{
}

/**
* Обработка системной ошибки
*/
void AsyncStream::stderror()
{
	onError(strerror(errno));
}

/**
* Неблокирующее чтение из потока
*/
ssize_t AsyncStream::read(void *buf, size_t count)
{
	ssize_t r = ::read(fd, buf, count);
	if ( r < 0 ) stderror();
	return r;
}

/**
* Неблокирующая запись в поток
*/
ssize_t AsyncStream::write(const void *buf, size_t count)
{
	ssize_t r = ::write(fd, buf, count);
	if ( r < 0 ) stderror();
	return r;
}

/**
* Приостановить чтение из потока
*/
bool AsyncStream::suspend()
{
	onError("TODO AsyncStream::suspend()");
}

/**
* Возобновить чтение из потока
*/
bool AsyncStream::resume()
{
	onError("TODO AsyncStream::resume()");
}

/**
* Закрыть поток
*/
void AsyncStream::close()
{
	int r = ::close(fd);
	if ( r < 0 ) stderror();
}

/**
* Событие ошибки
*
* Вызывается в случае возникновения какой-либо ошибки.
* По умолчанию выводит все ошибки в stderr
*/
void AsyncStream::onError(const char *message)
{
	cerr << "[AsyncStream]: " << message << endl;
}
