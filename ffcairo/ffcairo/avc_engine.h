#ifndef AVC_ENGINE_H
#define AVC_ENGINE_H

#include <nanosoft/netdaemon.h>

class AVCEngine
{
public:
	/**
	 * Ссылка на движок NetDaemon
	 */
	NetDaemon *daemon;
	
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
