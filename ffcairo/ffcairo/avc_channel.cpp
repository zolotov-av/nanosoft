
#include <ffcairo/avc_channel.h>

/**
* Конструктор
*/
AVCChannel::AVCChannel(int afd, AVCEngine *e): AsyncStream(afd), engine(e)
{
}

/**
* Деструктор
*/
AVCChannel::~AVCChannel()
{
}

/**
* Обработчик прочитанных данных
*/
void AVCChannel::onRead(const char *data, size_t len)
{
	// TODO
}

/**
* Пир (peer) закрыл поток.
*
* Мы уже ничего не можем отправить в ответ,
* можем только корректно закрыть соединение с нашей стороны.
*/
void AVCChannel::onPeerDown()
{
	// TODO
}
