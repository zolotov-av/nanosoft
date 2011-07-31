
#include <nanosoft/syslog/syslogserver.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>

static char message[4096];

SyslogServer::SyslogServer()
{
}

SyslogServer::~SyslogServer()
{
}

/**
* Принять входящее соединение
*/
void SyslogServer::onRead()
{
	socklen_t len;
	struct sockaddr_in addr;
	char ip[80];
	memset(&addr, 0, sizeof(struct sockaddr_in));
	int status = recvfrom(fd, &message, sizeof(message), 0, (struct sockaddr*)&addr, &len);
	if ( status == -1 )
	{
		stderror();
		return;
	}
	addr.sin_addr.s_addr = htonl(ntohl(addr.sin_addr.s_addr) & 0xFFFF0000);
	inet_ntop(AF_INET, &(addr.sin_addr), ip, 80);
	message[status] = 0;
	onMessage(ip, message);
}

/**
* Сигнал завершения работы
*
* Сервер решил закрыть соединение, здесь ещё есть время
* корректно попрощаться с пиром (peer).
*/
void SyslogServer::onTerminate()
{
	fprintf(stderr, "SyslogServer::onTerminate()\n");
}
