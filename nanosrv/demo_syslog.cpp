
#include <nanosoft/core/netdaemon.h>
#include <nanosoft/syslog/syslogserver.h>

#include <stdio.h>

class MySyslogServer: public SyslogServer
{
protected:
	/**
	* Обработка очередного log-сообщения
	*/
	virtual void onMessage(const char *ip, const char *message)
	{
		printf("[%s]: %s\n", ip, message);
	}
};

int main(int argc, char **argv)
{
	NetDaemon daemon(100);
	
	int port = 514;
	if ( argc < 2 )
	{
		printf("default port is %d\n", port);
		printf("to switch other port use\n# demo_syslog <port>\n");
	}
	else
	{
		sscanf(argv[1], "%d", &port);
		printf("listen port: %d\n", port);
	}
	
	ptr<SyslogServer> syslog = new MySyslogServer();
	syslog->bind(port);
	
	daemon.addObject(syslog);
	printf("NetDaemon has %d objects\n", daemon.getObjectCount());
	daemon.run();
	return 0;
}
