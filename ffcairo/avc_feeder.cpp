/****************************************************************************

Клиент (фидер) для сервера видеоконференций

Фидер это клиент который отправляет на сервер свой видеопоток для участия
в видеоконференции, но при этом не является полноценным клиентом. Этот клиент
нам нужен на раннем этапе для тестирования и отладки, т.к. полноценной
инфраструктуры у нас пока нет.

****************************************************************************/

#include <nanosoft/logger.h>
#include <nanosoft/debug.h>
#include <nanosoft/netdaemon.h>
#include <nanosoft/asyncdns.h>
#include <ffcairo/avc_engine.h>
#include <ffcairo/avc_listen.h>
#include <ffcairo/ffctypes.h>
#include <ffcairo/avc_http.h>
#include <ffcairo/avc_agent.h>

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

void onTimer(const timeval &tv, NetDaemon* daemon)
{
	int ts = tv.tv_sec;
	static int old_ts = 0;
	if ( ts > old_ts )
	{
		old_ts = ts;
		
		logger.information("avc_server is working, uptime: %lu seconds", logger.uptime());
	}
}

int main(int argc, char** argv)
{
	logger.open("avc_feeder.log");
	logger.information("avc_feeder start");
	
	// загружаем опции отладки из переменных окружения
	DEBUG::load_from_env();
	
	parse_options(argc, argv);
	
	if ( opts.fork ) forkDaemon();
	
	// INIT
	av_register_all();
	
	NetDaemon daemon(100, 1024);
	ptr<AVCAgent> avc_agent = new AVCAgent(&daemon);
	avc_agent->createSocket();
	avc_agent->connectTo("127.0.0.1", 8001);
	avc_agent->enableObject();
	
	adns = new AsyncDNS(&daemon);
	
	daemon.addObject(adns);
	daemon.addObject(avc_agent);
	
	daemon.setGlobalTimer(onTimer, &daemon);
	
	avc_payload_t p;
	p.type = AVC_SIMPLE;
	p.channel = 1;
	const char *s = "Hello world";
	int plen = strlen(s) + 5;
	strcpy(p.buf, s);
	avc_set_packet_len(&p, plen);
	printf("plen=%d, avc_packet_len()=%d\n", plen, avc_packet_len(&p));
	avc_agent->sendPacket(&p);
	
	daemon.run();
	
	logger.information("avc_feeder exit");
	printf("avc_feeder exit\n");
	return 0;
}
