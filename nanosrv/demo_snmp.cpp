
#include <nanosoft/core/netdaemon.h>
#include <nanosoft/snmp/snmpserver.h>

#include <stdio.h>

const int oid[] = {1,3,6,1,2,1,2,2,1,1,1024};

int main(int argc, char **argv)
{
	NetDaemon daemon(100);
	
	int port = 161;
	const char *community = "public";
	
	if ( argc < 2 )
	{
		printf("default port is %d\n", port);
		printf("default community is '%s'\n", community);
		printf("to switch other port/community use\n# demo_snmp <community> <port>\n");
	}
	else
	{
		if ( argc < 3 )
		{
			community = argv[1];
			printf("default port is %d\n", port);
			printf("community is '%s'\n", community);
			printf("to switch other port/community use\n# demo_snmp <community> <port>\n");
		}
		else
		{
			community = argv[1];
			sscanf(argv[2], "%d", &port);
			printf("listen port: %d\n", port);
			printf("community is '%s'\n", community);
			printf("to switch other port/community use\n# demo_snmp <community> <port>\n");
		}
	}
	
	SNMPServer *snmp = new SNMPServer();
	snmp->bind(port);
	snmp->sendGET("192.169.13.1", community, oid, sizeof(oid)/sizeof(oid[0]));
	
	daemon.addObject(snmp);
	printf("NetDaemon has %d objects\n", daemon.getObjectCount());
	daemon.run();
	return 0;
}
