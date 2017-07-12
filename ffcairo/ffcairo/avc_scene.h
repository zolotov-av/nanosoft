#ifndef AVC_SCENE_H
#define AVC_SCENE_H

#include <nanosoft/object.h>

#include <ffcairo/ffcimage.h>
#include <ffcairo/scale.h>
#include <ffcairo/ffcmuxer.h>


/**
 * Класс описывающий сцену
 *
 * Это "канва" на которую будет "рисоваться" контент, этот же класс отвечает
 * за компрессию данных
 */
class AVCScene: public FFCMuxer
{
public:
	
	/**
	 * Опции видео-кодека
	 */
	FFCVideoOptions opts;
	
	/**
	 * Номер фрейма
	 */
	int iFrame;
	
	/**
	 * Хост на котором будет рисоваться видео
	 */
	ptr<FFCImage> pic;
	
	/**
	 * Контест маштабирования
	 */
	ptr<FFCScale> scale;
	
	/**
	 * Видео-поток
	 */
	ptr<FFCVideoOutput> vo;
	
	/**
	 * контекст AVIO
	 */
	AVIOContext *avio_ctx;
	
	/**
	 * Конструктор
	 */
	AVCScene();
	
	/**
	 * Деструктор
	 */
	~AVCScene();
	
protected:
	
	/**
	 * Обработчик записи пакета в поток
	 */
	static int write_packet(void *opaque, uint8_t *buf, int buf_size);
	
	/**
	 * Отправить фрейм в кодировщик
	 */
	int sendFrame();
	
	/**
	 * Отправить пакет в стрим
	 */
	int sendPacket(AVPacket *pkt);
	
public:
	
	/**
	 * Создать контекст с AVIO
	 */
	int createAVIO(const char *format);
	
	/**
	 * Инициализация стриминга
	 */
	int initStreaming(int width, int height, int64_t bit_rate);
	
	/**
	 * Завершить стриминг
	 *
	 * Временная функция для прерывания стрима в случае непредвиденных ошибок
	 */
	void failStreaming();
	
	/**
	 * Выпустить новый фрейм и отправить его в стриминг
	 */
	void emitFrame();
	
	/**
	 * Временный таймер
	 */
	void onTimer();
};

#endif // AVC_SCENE_H