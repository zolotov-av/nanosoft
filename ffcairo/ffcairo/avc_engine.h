#ifndef AVC_ENGINE_H
#define AVC_ENGINE_H

#include <nanosoft/netdaemon.h>

class AVCHttp;

class AVCEngine
{
public:
	/**
	 * Ссылка на движок NetDaemon
	 */
	NetDaemon *daemon;
	
	/**
	 * временная ссылка на http-поток, по хорошему надо список
	 */
	ptr<AVCHttp> http;
	
	/**
	 * Конструктор
	 */
	AVCEngine(NetDaemon *d);
	
	/**
	 * Деструктор
	 */
	~AVCEngine();
};

#endif // AVC_ENGINE_H
