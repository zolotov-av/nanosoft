#ifndef AVC_CHANNEL_H
#define AVC_CHANNEL_H

#include <ffcairo/avc_stream.h>
#include <ffcairo/avc_engine.h>
#include <ffcairo/ffcdemuxer.h>

#define MAX_PACKET_COUNT 5000

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
	
	enum {
		/**
		 * Ожидание фида
		 */
		FEED_WAIT,
		
		/**
		 * Открытие фида
		 */
		FEED_OPEN_INPUT,
		
		/**
		 * Чтение тела
		 */
		FEED_STREAMING,
		
		/**
		 * Состояние ошибки
		 */
		FEED_FAILED
	} feed_state;
	
	/**
	 * Буфер пакетов
	 */
	avc_payload_t *pkt_buf[MAX_PACKET_COUNT];
	
	/**
	 * Число пакетов в буфере
	 */
	int pkt_count;
	
	int queue_size;
	
	int init_size;
	
	/**
	 * Демуксер
	 */
	ptr<FFCDemuxer> demux;
	
	/**
	 * контекст AVIO
	 */
	AVIOContext *avio_ctx;
	
	/**
	 * Конструктор
	 */
	AVCChannel(int afd, AVCEngine *e);
	
	/**
	 * Деструктор
	 */
	virtual ~AVCChannel();
	
//protected:
public:
	
	/**
	 * Обработчик чтения пакета из потока
	 */
	static int read_packet(void *opaque, uint8_t *buf, int buf_size);
	
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
	
	/**
	 * буферизовать пакет
	 */
	void queuePacket(const avc_packet_t *pkt);
	
	/**
	 * прочитать данные из очереди
	 */
	int queueRead(uint8_t *buf, int buf_size);
	
	/**
	 * Обработка данных в фида
	 */
	void handleFeedData();
	
public:
	
	/**
	 * Обработчик kick'а
	 *
	 * Временная, а может и постоянная функция, вызывается когда фидер был
	 * замещен новым потоком. Здесь мы завершаем свой поток и дисконнектим
	 * клиента
	 */
	void handleKick();
	
	/**
	 * Обработчик нового фидера
	 *
	 * Вызывается когда от клиента получает соответвующую команду, здесь
	 * нужно инициализировать демуксер и подключиться к сцене
	 */
	void handleNewFeed();
	
	/**
	 * Открыть входящий поток
	 */
	bool openFeed();
};

#endif // AVC_CHANNEL_H
