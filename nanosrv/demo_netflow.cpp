
#include <nanosoft/core/netdaemon.h>
#include <nanosoft/netflow/netflowserver.h>

#include <stdio.h>

int main(int argc, char **argv)
{
	NetDaemon daemon(100);
	
	int port = 5000;
	if ( argc < 2 )
	{
		printf("default port is %d\n", port);
		printf("to switch other port use\n# demo_netflow <port>\n");
	}
	else
	{
		sscanf(argv[1], "%d", &port);
		printf("listen port: %d\n", port);
	}
	
	ptr<NetFlowServer> nfserver = new NetFlowServer();
	nfserver->collector->addClass("in");
	nfserver->collector->addClassFilter("0.0.0.0/0", "46.175.120.0/21", "in");
	nfserver->collector->addClassFilter("0.0.0.0/0", "31.41.184.0/21", "in");
	nfserver->collector->addClass("out");
	nfserver->collector->addClassFilter("46.175.120.0/21", "0.0.0.0/0", "out");
	nfserver->collector->addClassFilter("31.41.184.0/21", "0.0.0.0/0", "out");
	nfserver->collector->addClass("loc");
	nfserver->collector->addClassFilter("46.175.120.0/21", "46.175.120.0/21", "loc");
	nfserver->collector->addClassFilter("31.41.184.0/21", "46.175.120.0/21", "loc");
	nfserver->collector->addClassFilter("46.175.120.0/21", "31.41.184.0/21", "loc");
	nfserver->collector->addClassFilter("31.41.184.0/21", "31.41.184.0/21", "loc");
	nfserver->bind(port);
	
	daemon.addObject(nfserver);
	printf("NetDaemon has %d objects\n", daemon.getObjectCount());
	daemon.run();
	return 0;
}
