#ifndef NANOSOFT_ASYNCSTREAM_H
#define NANOSOFT_ASYNCSTREAM_H

#include <sys/types.h>
#include <nanosoft/asyncobject.h>
#include <nanosoft/config.h>

#ifdef HAVE_LIBZ
#include <zlib.h>
#endif // HAVE_LIBZ

/**
* Название метода компрессии
*/
typedef const char *compression_method_t;

/**
* Класс для асинхронной работы с потоками
*/
class AsyncStream: public AsyncObject
{
private:
	/**
	* Флаги
	*/
	int flags;
	
#ifdef HAVE_LIBZ
	/**
	* Флаг компрессии zlib
	*
	* TRUE - компрессия включена
	* FALSE - компрессия отключена
	*/
	bool compression;
	
	/**
	* Контекст компрессора zlib исходящего трафика
	*/
	z_stream strm_tx;
	
	/**
	* Контекст декомпрессора zlib входящего трафика
	*/
	z_stream strm_rx;
	
	/**
	* Обработка поступивших сжатых данных
	*/
	void handleInflate(const char *data, size_t len);
	
	/**
	* Записать данные со сжатием zlib deflate
	*
	* Данные сжимаются и записываются в файловый буфер. Данная функция
	* пытается принять все данные и возвращает TRUE если это удалось.
	*
	* TODO В случае неудачи пока не возможно определить какая часть данных
	* была записана, а какая утеряна.
	*
	* @param data указатель на данные
	* @param len размер данных
	* @return TRUE данные приняты, FALSE произошла ошибка
	*/
	bool putDeflate(const char *data, size_t len);
#endif // HAVE_LIBZ
	
	/**
	* Обработка поступивших данных
	* 
	* Данные читаются из сокета и при необходимости обрабатываются
	* (сжатие, шифрование и т.п.). Обработанные данные затем передаются
	* виртуальному методу onRead()
	*/
	void handleRead();
	
	/**
	* Обработка поступивших данных
	*
	* Данные уже прочтены и обработанны, это общая точка через которую
	* проходят обработанные данные перед вызовом onRead()
	*/
	void handleRead(const char *data, size_t len);
	
	/**
	* Отправка накопленных данных
	*
	* Эта функция вызывается когда сокет готов принять данные и производит
	* отправку данных накопленных в файловом буфере
	*/
	void handleWrite();
	
	/**
	* Записать данные без компрессии
	*
	* Данные записываются сначала в файловый буфер и только потом отправляются.
	* Для обеспечения целостности переданный блок либо записывается целиком
	* и функция возвращает TRUE, либо ничего не записывается и функция
	* возвращает FALSE
	*
	* @param data указатель на данные
	* @param len размер данных
	* @return TRUE данные приняты, FALSE данные не приняты - нет места
	*/
	bool putUncompressed(const char *data, size_t len);
	
protected:
	
	/**
	* Вернуть маску ожидаемых событий
	*/
	virtual uint32_t getEventsMask();
	
	/**
	* Обработчик события
	*/
	virtual void onEvent(uint32_t events);
	
	/**
	* Обработчик прочитанных данных
	*/
	virtual void onRead(const char *data, size_t len) = 0;
	
	/**
	* Пир (peer) закрыл поток.
	*
	* Мы уже ничего не можем отправить в ответ,
	* можем только корректно закрыть соединение с нашей стороны.
	*/
	virtual void onPeerDown() = 0;
public:
	
	/**
	* Для shutdown()
	*/
	enum { READ = 1, WRITE = 2 };
	
	/**
	* Конструктор
	*/
	AsyncStream(int afd);
	
	/**
	* Деструктор
	*/
	virtual ~AsyncStream();
	
	/**
	* Проверить поддерживается ли компрессия
	* @return TRUE - компрессия поддерживается, FALSE - компрессия не поддерживается
	*/
	bool canCompression();
	
	/**
	* Проверить поддерживается ли компрессия конкретным методом
	* @param method метод компрессии
	* @return TRUE - компрессия поддерживается, FALSE - компрессия не поддерживается
	*/
	bool canCompression(const char *method);
	
	/**
	* Вернуть список поддерживаемых методов компрессии
	*/
	const compression_method_t* getCompressionMethods();
	
	/**
	* Вернуть флаг компрессии
	* @return TRUE - компрессия включена, FALSE - компрессия отключена
	*/
	bool isCompressionEnable();
	
	/**
	* Вернуть текущий метод компрессии
	* @return имя метода компрессии или NULL если компрессия не включена
	*/
	compression_method_t getCompressionMethod();
	
	/**
	* Включить компрессию
	* @param method метод компрессии
	* @return TRUE - компрессия включена, FALSE - компрессия не включена
	*/
	bool enableCompression(compression_method_t method);
	
	/**
	* Отключить компрессию
	* @return TRUE - компрессия отключена, FALSE - произошла ошибка
	*/
	bool disableCompression();
	
	/**
	* Записать данные
	*
	* При необходимости данные обрабатываются (сжимаются, шифруются и т.п.)
	* и записываются в файловый буфер. Фактическая отправка данных будет
	* когда сокет готов будет принять очередную порцию данных. Эта функция
	* старается принять все переданные данные и возвращает TRUE если это удалось.
	* Если принять данные не удалось, то возвращается FALSE. В случае
	* использования сжатия и/или компрессии невозможно точно установить
	* какая часть данных была записана в буфер.
	*
	* @param data указатель на данные
	* @param len размер данных
	* @return TRUE данные приняты, FALSE данные не приняты - нет места
	*/
	bool put(const char *data, size_t len);
	
	/**
	* Завершить чтение/запись
	* @note только для сокетов
	*/
	bool shutdown(int how);
	
	/**
	* Закрыть поток
	*/
	void close();
};

#endif // NANOSOFT_ASYNCSTREAM_H
