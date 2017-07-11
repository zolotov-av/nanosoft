#ifndef AVC_CHANNEL_H
#define AVC_CHANNEL_H

#include <ffcairo/avc_stream.h>
#include <ffcairo/avc_engine.h>

/**
 * Канал (сетевой) сервера видеоконференций
 *
 * Данный класс описывают серверную часть канала
 */
class AVCChannel: public AVCStream
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
	 * Пир (peer) закрыл поток.
	 *
	 * Мы уже ничего не можем отправить в ответ,
	 * можем только корректно закрыть соединение с нашей стороны.
	 */
	virtual void onPeerDown();
	
	/**
	 * Обработчик пакета
	 */
	virtual void onPacket(const avc_packet_t *pkt);
};

#endif // AVC_CHANNEL_H
