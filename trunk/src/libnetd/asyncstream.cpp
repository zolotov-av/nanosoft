#include <sys/types.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <nanosoft/asyncstream.h>
#include <nanosoft/netdaemon.h>
#include <nanosoft/error.h>
#include <nanosoft/config.h>
#include <sys/socket.h>

using namespace std;

/**
* Конструктор
*/
AsyncStream::AsyncStream(int afd): AsyncObject(afd), flags(0)
{
#ifdef HAVE_LIBZ
	compression = false;
#endif // HAVE_LIBZ
}

/**
* Деструктор
*/
AsyncStream::~AsyncStream()
{
	printf("AsyncStream[%d]: deleting\n", getFd());
	close();
	disableCompression();
}

/**
* Обработка поступивших данных
*/
void AsyncStream::handleRead()
{
	printf("AsyncStream[%d]: handleRead\n", getFd());
	
	char chunk[FD_READ_CHUNK_SIZE];
	ssize_t r = ::read(getFd(), chunk, sizeof(chunk));
	while ( r > 0 )
	{
#ifdef HAVE_LIBZ
		if ( compression ) handleInflate(chunk, r);
		else handleRead(chunk, r);
#else
		handleRead(chunk, r);
#endif // HAVE_LIBZ
		r = ::read(getFd(), chunk, sizeof(chunk));
	}
	if ( r < 0 ) stderror();
}

#ifdef HAVE_LIBZ
/**
* Обработка поступивших сжатых данных
*/
void AsyncStream::handleInflate(const char *data, size_t len)
{
	printf("AsyncStream[%d]: handleInflate\n", getFd());
	
	char buf[ZLIB_INFLATE_CHUNK_SIZE];
	
	strm_rx.next_in = (unsigned char*)data;
	strm_rx.avail_in = len;
	
	while ( strm_rx.avail_out == 0 )
	{
		strm_rx.next_out = (unsigned char*)buf;
		strm_rx.avail_out = sizeof(buf);
		
		inflate(&strm_rx, Z_SYNC_FLUSH);
		
		size_t have = sizeof(buf) - strm_rx.avail_out;
		
		string s(buf, have);
		printf("inlated: %s\n", s.c_str());
		handleRead(buf, have);
	}
	
	strm_rx.avail_out = 0;
}
#endif // HAVE_LIBZ

/**
* Обработка поступивших данных после распаковки
*/
void AsyncStream::handleRead(const char *data, size_t len)
{
#ifdef DUMP_IO
	string io_dump(data, len);
	fprintf(stdout, "DUMP READ[%d]: \033[22;32m%s\033[0m\n", getFd(), io_dump.c_str());
#endif
	onRead(data, len);
}

/**
* Отправка накопленных данных
*/
void AsyncStream::handleWrite()
{
	printf("AsyncStream[%d]: handleWrite\n", getFd());
	getDaemon()->push(getFd());
}

/**
* Вернуть маску ожидаемых событий
*/
uint32_t AsyncStream::getEventsMask()
{
	return EPOLLIN | EPOLLRDHUP | EPOLLONESHOT | EPOLLHUP | EPOLLERR;
}

/**
* Обработчик события
*/
void AsyncStream::onEvent(uint32_t events)
{
	if ( events & EPOLLERR ) onError("epoll report some error in stream...");
	if ( events & EPOLLIN ) handleRead();
	if ( events & EPOLLOUT ) handleWrite();
	if ( (events & EPOLLRDHUP) || (events & EPOLLHUP) ) onPeerDown();
}

/**
* Проверить поддерживается ли компрессия
* @return TRUE - компрессия поддерживается, FALSE - компрессия не поддерживается
*/
bool AsyncStream::canCompression()
{
#ifdef HAVE_LIBZ
	return true;
#else
	return false;
#endif // HAVE_LIBZ
}

/**
* Проверить поддерживается ли компрессия конкретным методом
* @param method метод компрессии
* @return TRUE - компрессия поддерживается, FALSE - компрессия не поддерживается
*/
bool AsyncStream::canCompression(const char *method)
{
#ifdef HAVE_LIBZ
	return strcmp(method, "zlib") == 0;
#else
	return false;
#endif // HAVE_LIBZ
}

/**
* Вернуть список поддерживаемых методов компрессии
*/
const compression_method_t* AsyncStream::getCompressionMethods()
{
	static compression_method_t methods[] = {
#ifdef HAVE_LIBZ
		"zlib",
#endif // HAVE_LIBZ
		0
	};
	return methods;
}

/**
* Вернуть флаг компрессии
* @return TRUE - компрессия включена, FALSE - компрессия отключена
*/
bool AsyncStream::isCompressionEnable()
{
#ifdef HAVE_LIBZ
	return compression;
#else
	return false;
#endif // HAVE_LIBZ
}

/**
* Вернуть текущий метод компрессии
* @return имя метода компрессии или NULL если компрессия не включена
*/
compression_method_t AsyncStream::getCompressionMethod()
{
#ifdef HAVE_LIBZ
	return compression ? "zlib" : 0;
#else
	return 0;
#endif // HAVE_LIBZ
}

/**
* Включить компрессию
* @param method метод компрессии
* @return TRUE - компрессия включена, FALSE - компрессия не включена
*/
bool AsyncStream::enableCompression(compression_method_t method)
{
#ifdef HAVE_LIBZ
	if ( ! compression && canCompression(method) )
	{
		int status;
		
		// инициализация компрессора исходящего трафика
		memset(&strm_tx, 0, sizeof(strm_tx));
		status = deflateInit(&strm_tx, ZLIB_COMPRESS_LEVEL);
		if ( status != Z_OK )
		{
			(void)deflateEnd(&strm_tx);
			return false;
		}
		
		// инициализация декомпрессора входящего трафика
		memset(&strm_rx, 0, sizeof(strm_rx));
		status = inflateInit(&strm_rx);
		if ( status != Z_OK )
		{
			(void)deflateEnd(&strm_tx);
			(void)inflateEnd(&strm_rx);
			return false;
		}
		
		compression = true;
		return true;
	}
	return false;
#else
	return false;
#endif // HAVE_LIBZ
}

/**
* Отключить компрессию
* @return TRUE - компрессия отключена, FALSE - произошла ошибка
*/
bool AsyncStream::disableCompression()
{
#ifdef HAVE_LIBZ
	if ( compression )
	{
		(void)deflateEnd(&strm_tx);
		(void)inflateEnd(&strm_rx);
		
		compression = false;
	}
#else
	return true;
#endif // HAVE_LIBZ
}

#ifdef HAVE_LIBZ
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
bool AsyncStream::putDeflate(const char *data, size_t len)
{
	printf("AsyncStream[%d] deflate\n", getFd());
	
	size_t out_len = 0;
	char buf[ZLIB_DEFLATE_CHUNK_SIZE];
	
	strm_tx.next_in = (unsigned char*)data;
	strm_tx.avail_in = len;
	
	while ( strm_tx.avail_out == 0 )
	{
		strm_tx.next_out = (unsigned char*)buf;
		strm_tx.avail_out = sizeof(buf);
		
		deflate(&strm_tx, Z_PARTIAL_FLUSH);
		
		size_t have = sizeof(buf) - strm_tx.avail_out;
		out_len += have;
		
		if ( ! putUncompressed(buf, have) ) return false;
	}
	
	strm_tx.avail_out = 0;
	
	float ratio = static_cast<float>(len) / out_len;
	printf("compression ratio: %d / %d = %.2f\n", len, out_len, ratio);
	
	return true;
}
#endif // HAVE_LIBZ

/**
* Записать данные
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
bool AsyncStream::put(const char *data, size_t len)
{
#ifdef DUMP_IO
	string io_dump(data, len);
	fprintf(stdout, "DUMP WRITE[%d]: \033[22;34m%s\033[0m\n", getFd(), io_dump.c_str());
#endif
	
#ifdef HAVE_LIBZ
	if ( compression )
	{
		return putDeflate(data, len);
	}
#endif // HAVE_LIBZ
	return putUncompressed(data, len);
}

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
bool AsyncStream::putUncompressed(const char *data, size_t len)
{
	NetDaemon *daemon = getDaemon();
	printf("AsyncStream[%d, %p] put uncompressed\n", getFd(), daemon);
	if ( daemon )
	{
		if ( daemon->put(getFd(), data, len) )
		{
			daemon->modifyObject(this);
			return true;
		}
	}
	return false;
}

/**
* Завершить чтение/запись
* @note только для сокетов
*/
bool AsyncStream::shutdown(int how)
{
	printf("AsyncStream[%d] shutdown\n", getFd());
	if ( how & READ & ~ flags ) {
		if ( ::shutdown(getFd(), SHUT_RD) != 0 ) stderror();
		flags |= READ;
	}
	if ( how & WRITE & ~ flags ) {
		if ( ::shutdown(getFd(), SHUT_WR) != 0 ) stderror();
		flags |= WRITE;
	}
}

/**
* Закрыть поток
*/
void AsyncStream::close()
{
	if ( getFd() )
	{
		int r = ::close(getFd());
		setFd(0);
		if ( r < 0 ) stderror();
	}
}
