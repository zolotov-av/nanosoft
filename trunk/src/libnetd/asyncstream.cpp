#include <sys/types.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <nanosoft/asyncstream.h>

using namespace std;

/**
* Конструктор
*/
AsyncStream::AsyncStream(int afd): AsyncObject(afd)
{
}

/**
* Деструктор
*/
AsyncStream::~AsyncStream()
{
}

/**
* Вернуть маску ожидаемых событий
*/
uint32_t AsyncStream::getEventsMask()
{
	return EPOLLIN | EPOLLRDHUP | EPOLLONESHOT | EPOLLHUP | EPOLLERR;
}

/**
* Обработчик события
*/
void AsyncStream::onEvent(uint32_t events)
{
	if ( events & EPOLLERR ) onError("epoll report some error in stream...");
	else if ( events & EPOLLIN ) onRead();
	else if ( (events & EPOLLRDHUP) || (events & EPOLLHUP) ) onShutdown();
	else onError("----------");
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
