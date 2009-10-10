#include <sys/types.h>
#include <unistd.h>
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
* Неблокирующее чтение из потока
*/
ssize_t AsyncStream::read(void *buf, size_t count)
{
	return ::read(fd, buf, count);
}

/**
* Неблокирующая запись в поток
*/
ssize_t AsyncStream::write(const void *buf, size_t count)
{
	return ::write(fd, buf, count);
}

/**
* Приостановить чтение из потока
*/
bool AsyncStream::suspend()
{
	cerr << "TODO AsyncStream::suspend()" << endl;
}

/**
* Возобновить чтение из потока
*/
bool AsyncStream::resume()
{
	cerr << "TODO AsyncStream::resume()" << endl;
}

/**
* Закрыть поток
*/
void AsyncStream::close()
{
	::close(fd);
}
