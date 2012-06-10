#include <sys/types.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <nanosoft/asyncstream.h>
#include <nanosoft/netdaemon.h>
#include <nanosoft/error.h>
#include <nanosoft/config.h>
#include <sys/socket.h>

using namespace std;

/**
* Конструктор
*/
AsyncStream::AsyncStream(int afd): AsyncObject(afd), flags(0)
{
}

/**
* Деструктор
*/
AsyncStream::~AsyncStream()
{
	printf("AsyncStream[%d]: deleting\n", getFd());
	close();
}

/**
* Обработка поступивших данных
*/
void AsyncStream::handleRead()
{
	printf("AsyncStream[%d]: handleRead\n", getFd());
	
	char chunk[FD_READ_CHUNK_SIZE];
	ssize_t r = ::read(getFd(), chunk, sizeof(chunk));
	while ( r > 0 )
	{
		onRead(chunk, r);
		r = ::read(getFd(), chunk, sizeof(chunk));
	}
	if ( r < 0 ) stderror();
}

/**
* Отправка накопленных данных
*/
void AsyncStream::handleWrite()
{
	printf("AsyncStream[%d]: handleWrite\n", getFd());
	getDaemon()->push(getFd());
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
	if ( events & EPOLLIN ) handleRead();
	if ( events & EPOLLOUT ) handleWrite();
	if ( (events & EPOLLRDHUP) || (events & EPOLLHUP) ) onPeerDown();
}

/**
* Записать данные
*
* Данные записываются сначала в файловый буфер и только потом отправляются.
* Для обеспечения целостности переданный блок либо записывается целиком
* и функция возвращает TRUE, либо ничего не записывается и функция
* возвращает FALSE
*
* @param data указатель на данные
* @param len размер данных
* @return TRUE данные приняты, FALSE данные не приняты - нет места
*/
bool AsyncStream::put(const char *data, size_t len)
{
	NetDaemon *daemon = getDaemon();
	printf("AsyncStream[%d, %p] put\n", getFd(), daemon);
	if ( daemon )
	{
		if ( daemon->put(getFd(), data, len) )
		{
			daemon->modifyObject(this);
			return true;
		}
	}
	return false;
}

/**
* Завершить чтение/запись
* @note только для сокетов
*/
bool AsyncStream::shutdown(int how)
{
	printf("AsyncStream[%d] shutdown\n", getFd());
	if ( how & READ & ~ flags ) {
		if ( ::shutdown(getFd(), SHUT_RD) != 0 ) stderror();
		flags |= READ;
	}
	if ( how & WRITE & ~ flags ) {
		if ( ::shutdown(getFd(), SHUT_WR) != 0 ) stderror();
		flags |= WRITE;
	}
}

/**
* Закрыть поток
*/
void AsyncStream::close()
{
	if ( getFd() )
	{
		int r = ::close(getFd());
		setFd(0);
		if ( r < 0 ) stderror();
	}
}
