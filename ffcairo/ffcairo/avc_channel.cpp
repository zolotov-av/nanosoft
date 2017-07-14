
#include <ffcairo/avc_channel.h>
#include <ffcairo/avc_scene.h>

/**
* Конструктор
*/
AVCChannel::AVCChannel(int afd, AVCEngine *e): AVCStream(afd), engine(e)
{
	printf("new AVCChannel\n");
	
	feed_state = FEED_WAIT;
	pkt_count = 0;
	
	queue_size = 0;
	init_size = 0;
}

/**
* Деструктор
*/
AVCChannel::~AVCChannel()
{
	printf("AVCChannel destroyed\n");
}

static int real_read_packet(AVCChannel *ch, uint8_t *buf, int buf_size)
{
	printf("real_read_packet(%d)\n", buf_size);
	int len = 0;
	while ( len < buf_size )
	{
		int old_qsize = ch->queue_size;
		int ret = ch->queueRead(buf + len, buf_size - len);
		printf("queueRead() = %d, old_qsize=%d, new_qsize=%d\n", ret, old_qsize, ch->queue_size);
		if ( ret < 0 )
		{
			//if ( len == 0 ) return AVERROR(EAGAIN);
			return len;
		}
		ch->init_size += ret;
		len += ret;
	}
	return len;
}

/**
* Обработчик чтения пакета из потока
*/
int AVCChannel::read_packet(void *opaque, uint8_t *buf, int buf_size)
{
	AVCChannel *ch = (AVCChannel*)opaque;
	if ( ch )
	{
		int old_qsize = ch->queue_size;
		int ret = real_read_packet(ch, buf, buf_size);
		printf("read_packet(%d) = %d, old_qsize=%d, new_qszie=%d\n", buf_size, ret, old_qsize, ch->queue_size);
		if ( ret == 0 ) return AVERROR(EAGAIN);
		return ret;
	}
	return AVERROR(EAGAIN);
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
	//printf("onPacket() type=%d, channel=%d, size=%d, state=%d\n", pkt->type, pkt->channel, avc_packet_len(pkt), feed_state);
	
	if ( pkt->channel == 0 )
	{
		if ( pkt->type == 1 )
		{
			handleNewFeed();
			return;
		}
	}
	
	if ( pkt->channel == 2 && pkt->type == AVC_PAYLOAD )
	{
		if ( feed_state == FEED_OPEN_INPUT )
		{
			queuePacket(pkt);
			handleFeedData();
			return;
		}
		
		if ( feed_state == FEED_STREAMING )
		{
			queuePacket(pkt);
			handleFeedData();
			return;
		}
	}
}

/**
* буферизовать пакет
*/
void AVCChannel::queuePacket(const avc_packet_t *pkt)
{
	if ( pkt_count < MAX_PACKET_COUNT )
	{
		int plen = avc_packet_len(pkt);
		avc_payload_t *p = (avc_payload_t *)malloc(plen);
		if ( ! p )
		{
			printf("malloc() failed\n");
			exit(-1);
			return;
		}
		memcpy(p, pkt, plen);
		pkt_buf[pkt_count++] = p;
		queue_size += plen - 4;
		return;
	}
	
	printf("queue full\n");
	exit(-1);
	return;
}

/**
* прочитать данные из очереди
*/
int AVCChannel::queueRead(uint8_t *buf, int buf_size)
{
	if ( pkt_count <= 0 )
	{
		printf("    queue empty\n");
		return AVERROR(EAGAIN);
	}
	
	avc_payload_t *p = pkt_buf[0];
	int plen = avc_packet_len(p) - 4;
	
	// если запрошено больше чем есть в пакете, то сколько есть в пакете
	if ( buf_size > plen ) buf_size = plen;
	
	memcpy(buf, p->buf, buf_size);
	
	if ( buf_size < plen )
	{
		// если пакет прочитан не полностью, то сдвинуть данные в пакете
		int xlen = plen - buf_size; // сколько осталось в пакете
		avc_set_packet_len(p, xlen + 4);
		for(int i = 0; i < xlen; i++)
		{
			p->buf[i] = p->buf[i+buf_size];
		}
		
		queue_size -= buf_size;
		return buf_size;
	}
	
	// если пакет прочитан полностью, то удалить пакет и подвинуть указатели
	pkt_count--;
	for(int i = 0; i < pkt_count; i++)
	{
		pkt_buf[i] = pkt_buf[i+1];
	}
	free(p);
	//printf("packet dequeued, count=%d\n", pkt_count);
	
	queue_size -= buf_size;
	return buf_size;
}

/**
* Обработка данных в фида
*/
void AVCChannel::handleFeedData()
{
	//printf("AVCChannel::handleFeedData()\n");
	
	if ( feed_state == FEED_OPEN_INPUT )
	{
		if ( queue_size > (80 * 1020) )
		{
			printf("openFeed():\n");
			if ( ! openFeed() )
			{
				printf("fail to openFeed()\n");
				close();
				exit(-1);
				return;
			}
			
			printf("openFeed() done, init_size=%d\n", init_size);
			
			feed_state = FEED_STREAMING;
			//engine->scene->replaceFeed(this);
		}
	}
	
	if ( feed_state == FEED_STREAMING )
	{
		bool ret;
		do
		{
			static int i = 1;
			if ( queue_size > (32*1024) )
			{
				printf("readFrame #%d, queue_size=%d\n", i, queue_size);
				ret = demux->readFrame();
				if ( ret ) printf("readFrame(#%d) ok\n", i++);
			}
			else
			{
				printf("queue_size low = %d\n", queue_size);
				break;
			}
		}
		while ( ret );
		
		static int cnt = 0;
		cnt ++;
		//if ( cnt == 60 ) exit(-1);
	}
}

/**
* Обработчик kick'а
*
* Временная, а может и постоянная функция, вызывается когда фидер был
* замещен новым потоком. Здесь мы завершаем свой поток и дисконнектим
* клиента
*/
void AVCChannel::handleKick()
{
	printf("AVCChannel::handleKick()\n");
	close();
}

/**
* Обработчик нового фидера
*
* Вызывается когда от клиента получает соответвующую команду, здесь
* нужно инициализировать демуксер и подключиться к сцене
*/
void AVCChannel::handleNewFeed()
{
	printf("AVCChannel::handleNewFeed()\n");
	
	if ( feed_state == FEED_WAIT )
	{
		feed_state = FEED_OPEN_INPUT;
		return;
	}
}

/**
* Открыть входящий поток
*/
bool AVCChannel::openFeed()
{
	demux = new FFCDemuxer();
	
	uint8_t *avio_ctx_buffer = NULL;
	size_t avio_ctx_buffer_size = 4096;
	
	avio_ctx_buffer = (uint8_t*)av_malloc(avio_ctx_buffer_size);
	if (!avio_ctx_buffer)
	{
		printf("av_malloc failed\n");
		return false;
	}

	avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size, 0, this/*user pointer*/, &read_packet, NULL, NULL);
	if ( !avio_ctx )
	{
		printf("avio_alloc_context() failed\n");
		return false;
	}
	
	if ( ! demux->createContext() )
	{
		printf("demux->createContext() failed\n");
		return false;
	}
	
	// установим AVIOContext
	demux->avFormat->pb = avio_ctx;
	
	// ограничим probesize, пусть лучше быстрее сфейлиться, чем будет минуту
	// жрать ресурсы
	demux->avFormat->probesize = 10 * 1024;
	
	int ret = demux->openAVIO();
	if ( ret < 0 )
	{
		printf("demux->openAVIO() failed\n");
		return false;
	}
	
	return true;
}