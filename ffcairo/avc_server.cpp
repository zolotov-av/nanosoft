/****************************************************************************

Сервер видеоконференций

****************************************************************************/

#include <nanosoft/logger.h>
#include <nanosoft/debug.h>
#include <nanosoft/netdaemon.h>
#include <nanosoft/asyncdns.h>
#include <ffcairo/avc_engine.h>
#include <ffcairo/avc_listen.h>

#include <stdio.h>

struct opts_t
{
	bool fork;
};

opts_t opts;

/**
 * Парсер опций
 */
void parse_options(int argc, char** argv)
{
	// init default options
	opts.fork = false;
	
	for(int i = 1; i < argc; i++)
	{
		if ( strcmp(argv[i], "-f") == 0 )
		{
			opts.fork = true;
			continue;
		}
		
		fprintf(stderr, "unknown option [%s]\n", argv[i]);
	}
}

/**
 * Форкнуть демона
 */
void forkDaemon()
{
	printf("fork daemon\n");
	
	pid_t pid = fork();
	
	// Если полученный родительским процессом pid меньше 0, значит, форкнуться не удалось
	if ( pid < 0 )
	{
		logger.error("fork failure: %s", strerror(errno));
		return;
	}
	
	if ( pid == 0 )
	{
		// потомок
		return;
	}
	else
	{
		// родитель
		logger.information("daemon forked, PID: %u", pid);
		exit(0);
	}
}

int main(int argc, char** argv)
{
	logger.open("avc_server.log");
	logger.information("avc_server start");
	
	parse_options(argc, argv);
	
	if ( opts.fork ) forkDaemon();
	
	NetDaemon daemon(100, 1024);
	AVCEngine avc_engine;
	ptr<AVCListen> avc_listen = new AVCListen(&avc_engine);
	avc_listen->bind(8000);
	avc_listen->listen(10);
	
	adns = new AsyncDNS(&daemon);
	
	daemon.addObject(adns);
	daemon.addObject(avc_listen);
	
	//daemon.setGlobalTimer(onTimer, &daemon);
	
	daemon.run();
	
	logger.information("avc_server exit");
	printf("avc_server exit\n");
	return 0;
}
