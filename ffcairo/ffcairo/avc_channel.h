#ifndef AVC_CHANNEL_H
#define AVC_CHANNEL_H

#include <nanosoft/asyncstream.h>
#include <ffcairo/avc_engine.h>

/**
 * Канал (сетевой) сервера видеоконференций
 *
 * В этом классе будет реализован протокол общения клиента и сервера. Позже,
 * когда протокол более-менее сформируется, он скорее всего будет вынесен
 * в отдельный класс и здесь останется только высокоуровневые функции
 * управления каналом.
 */
class AVCChannel: public AsyncStream
{
public:
	/**
	 * Ссылка на движок
	 */
	AVCEngine *engine;
	
	/**
	 * Конструктор
	 */
	AVCChannel(int afd, AVCEngine *e);
	
	/**
	 * Деструктор
	 */
	virtual ~AVCChannel();
	
protected:
	
	/**
	 * Обработчик прочитанных данных
	 */
	virtual void onRead(const char *data, size_t len);
	
	/**
	 * Пир (peer) закрыл поток.
	 *
	 * Мы уже ничего не можем отправить в ответ,
	 * можем только корректно закрыть соединение с нашей стороны.
	 */
	virtual void onPeerDown();
};

#endif // AVC_CHANNEL_H
