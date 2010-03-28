
#include <nanosoft/asyncserver.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

using namespace std;

/**
* Конструктор
*/
AsyncServer::AsyncServer()
{
}

/**
* Деструктор
*/
AsyncServer::~AsyncServer()
{
	close();
}

/**
* Подключиться к порту
*/
bool AsyncServer::bind(int port)
{
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if ( fd == 0 )
	{
		stderror();
		return false;
	}
	
	int yes = 1;
	if ( setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) stderror();
	
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);
	
	if ( ::bind(fd, (sockaddr *)&addr, sizeof(addr)) != 0 )
	{
		stderror();
		close();
		return false;
	}
	
	return true;
}

/**
* Подключиться к unix-сокету
*/
bool AsyncServer::bind(const char *path)
{
	fd = socket(AF_LOCAL, SOCK_STREAM, 0);
	if ( fd == 0 )
	{
		stderror();
		return false;
	}
	
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, path, sizeof(addr.sun_path)-1);
	unlink(path);
	if ( ::bind(fd, (struct sockaddr *) &addr, sizeof(addr)) != 0 )
	{
		stderror();
		close();
		return false;
	}
	
	return true;
}

/**
* Начать слушать сокет
*/
bool AsyncServer::listen(int backlog)
{
	if ( ::listen(fd, backlog) != 0 )
	{
		stderror();
		return false;
	}
	return true;
}

/**
* Принять соединение
*/
int AsyncServer::accept()
{
	int sock = ::accept(fd, 0, 0);
	if ( sock > 0 ) return sock;
	stderror();
	return 0;
}

/**
* Закрыть сокет
*/
void AsyncServer::close()
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
uint32_t AsyncServer::getEventsMask()
{
	return EPOLLIN | EPOLLRDHUP | EPOLLONESHOT | EPOLLHUP | EPOLLERR;
}

/**
* Обработчик события
*/
void AsyncServer::onEvent(uint32_t events)
{
	if ( events & EPOLLERR ) onError("epoll report some error in stream...");
	if ( events & EPOLLIN ) onAccept();
}
