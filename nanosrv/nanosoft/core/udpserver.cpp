
#include <nanosoft/core/udpserver.h>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <stdio.h>

/**
* Конструктор
*/
UDPServer::UDPServer()
{
}

/**
* Деструктор
*/
UDPServer::~UDPServer()
{
	close();
}

/**
* Подключиться к порту
*/
bool UDPServer::bind(int port)
{
	fd = socket(PF_INET, SOCK_DGRAM, 0);
	if ( fd == 0 )
	{
		stderror();
		return false;
	}
	
	//int yes = 1;
	//if ( setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) stderror();
	
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	
	int status = ::bind(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
	if ( status == -1)
	{
		stderror();
		return false;
	}
	
	return true;
}

/**
* Закрыть сокет
*/
void UDPServer::close()
{
	if ( fd ) {
		int r = ::close(fd);
		fd = 0;
		if ( r != 0 ) stderror();
	}
}


/**
* Вернуть маску ожидаемых событий
*/
uint32_t UDPServer::getEventsMask()
{
	return EPOLLIN | EPOLLRDHUP | EPOLLONESHOT | EPOLLHUP | EPOLLERR;
}

/**
* Обработчик события
*/
void UDPServer::onEvent(uint32_t events)
{
	if ( events & EPOLLERR ) fprintf(stderr, "epoll report some error in stream...");
	if ( events & EPOLLIN ) onRead();
}
