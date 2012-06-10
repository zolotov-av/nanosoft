
#include <nanosoft/asyncdns.h>
#include <nanosoft/netdaemon.h>
#include <udns.h>
#include <sys/epoll.h>
#include <time.h>

/**
* Конструктор
*/
AsyncDNS::AsyncDNS(NetDaemon *aDaemon): daemon(aDaemon)
{
	dns_init(0, 1);
	setFd( dns_sock(0) );
}

/**
* Деструктор
*/
AsyncDNS::~AsyncDNS()
{
	dns_close(0);
}

/**
* Вернуть маску ожидаемых событий
*/
uint32_t AsyncDNS::getEventsMask()
{
	return EPOLLIN | EPOLLONESHOT;
}

/**
* Обработчик события
*/
void AsyncDNS::onEvent(uint32_t events)
{
	dns_ioevent(0, 0);
}

/**
* Сигнал завершения работы
*
* Объект должен закрыть файловый дескриптор
* и освободить все занимаемые ресурсы
*/
void AsyncDNS::onTerminate()
{
	daemon->removeObject(this);
	dns_reset(0);
}

/**
* Таймер
*/
void AsyncDNS::timer(int wid, void *data)
{
	time_t now = time(0);
	int expires = dns_timeouts(0, -1, now);
	if ( expires >= 0 ) {
		static_cast<NetDaemon*>(data)->setTimer(expires + now, AsyncDNS::timer, data);
	}
}

/**
* Отправить запрос на получение IPv4-адреса
*/
bool AsyncDNS::a4(const char *name, dns_query_a4_fn callback, void *data)
{
	struct dns_query *q = dns_submit_a4(0, name, 0, callback, data);
	if ( q == 0 ) return false;
	timer(0, daemon);
	return true;
}

/**
* Отправить запрос на получение SRV-записи
*/
bool AsyncDNS::srv(const char *name, const char *service, const char *protocol, dns_query_srv_fn callback, void *data)
{
	struct dns_query *q = dns_submit_srv(0, name, service, protocol, 0, callback, data);
	if ( q == 0 ) return false;
	timer(0, daemon);
	return true;
}
