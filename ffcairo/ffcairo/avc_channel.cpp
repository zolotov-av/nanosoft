
#include <ffcairo/avc_channel.h>

/**
* Конструктор
*/
AVCChannel::AVCChannel(int afd, AVCEngine *e): AVCStream(afd), engine(e)
{
	printf("new AVCChannel\n");
}

/**
* Деструктор
*/
AVCChannel::~AVCChannel()
{
	printf("AVCChannel destroyed\n");
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
	leaveDaemon();
}

/**
* Обработчик пакета
*/
void AVCChannel::onPacket(const avc_packet_t *pkt)
{
	if ( pkt->type == AVC_SIMPLE && pkt->channel == 1 )
	{
		const avc_payload_t *p = (const avc_payload_t*)pkt;
		printf("payload: %s\n", p->buf);
	}
}
