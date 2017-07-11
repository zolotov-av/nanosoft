#ifndef AVC_LISTEN_H
#define AVC_LISTEN_H

#include <nanosoft/asyncserver.h>
#include <ffcairo/avc_engine.h>

class AVCListen: public AsyncServer
{
public:
	/**
	 * Ссылка на движок
	 */
	AVCEngine *engine;
	
	/**
	 * Конструктор
	 */
	AVCListen(AVCEngine *e);
	
protected:
	
	/**
	* Принять входящее соединение
	*/
	virtual void onAccept();
};

#endif // AVC_LISTEN_H
