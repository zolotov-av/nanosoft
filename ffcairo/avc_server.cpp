/****************************************************************************

Сервер видеоконференций

****************************************************************************/

#include <nanosoft/logger.h>
#include <nanosoft/debug.h>
#include <nanosoft/netdaemon.h>
#include <nanosoft/asyncdns.h>
#include <ffcairo/avc_engine.h>
#include <ffcairo/avc_listen.h>
#include <ffcairo/ffctypes.h>
#include <ffcairo/avc_http.h>
#include <ffcairo/avc_scene.h>

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

AVCEngine *en;

void onTimer(const timeval &tv, NetDaemon* daemon)
{
	int ts = tv.tv_sec;
	static int old_ts = 0;
	if ( ts > old_ts )
	{
		old_ts = ts;
		
		logger.information("avc_server is working, uptime: %lu seconds", logger.uptime());
		
		en->scene->onTimer();
	}
}

int main(int argc, char** argv)
{
	// игнорировать SIGPIPE
	// Клиент может внезапно закрыть соединение когда мы пытаемся передать
	// данные, в этом случае генерируется сигнал SIGPIPE и по умолчанию он
	// прерывает нашу программу. Самое простое решение - игнорировать сигнал,
	// не думаю что здесь нужна какая-то более сложная обратка сигнала.
	// В случае если сигнал игнорируется то функция send/write() будет
	// просто завешаться с ошибкой, которые и так уже обрабатывает NetDaemon
	signal(SIGPIPE, SIG_IGN);
	
	logger.open("avc_server.log");
	logger.information("avc_server start");
	
	// загружаем опции отладки из переменных окружения
	DEBUG::load_from_env();
	
	parse_options(argc, argv);
	
	if ( opts.fork ) forkDaemon();
	
	// INIT
	av_register_all();
	
	NetDaemon daemon(100, 1024);
	AVCEngine avc_engine(&daemon);
	en = &avc_engine;
	
	int ret = avc_engine.scene->initStreaming(1280, 720, 2000000);
	if ( ret < 0 )
	{
		printf("initStreaming() failed");
		return -1;
	}
	
	ptr<AVCListen> avc_listen = new AVCListen(&avc_engine, AVC_CHANNEL);
	avc_listen->bind(8001);
	avc_listen->listen(10);
	
	ptr<AVCListen> avc_http = new AVCListen(&avc_engine, AVC_HTTP);
	avc_http->bind(8000);
	avc_http->listen(10);
	
	adns = new AsyncDNS(&daemon);
	
	daemon.addObject(adns);
	daemon.addObject(avc_http);
	daemon.addObject(avc_listen);
	
	daemon.setGlobalTimer(onTimer, &daemon);
	
	daemon.run();
	
	logger.information("avc_server exit");
	printf("avc_server exit\n");
	return 0;
}
