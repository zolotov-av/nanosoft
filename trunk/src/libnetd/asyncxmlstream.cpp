#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <nanosoft/asyncxmlstream.h>

using namespace std;

/**
* Конструктор
*/
AsyncXMLStream::AsyncXMLStream(int afd): AsyncStream(afd)
{
}

/**
* Деструктор
*/
AsyncXMLStream::~AsyncXMLStream()
{
}

/**
* Событие готовности к чтению
*
* Вызывается когда в потоке есть данные,
* которые можно прочитать без блокирования
*/
void AsyncXMLStream::onRead()
{
	char buf[4096];
	ssize_t r = read(buf, sizeof(buf));
	if ( r > 0 )
	{
		parseXML(buf, r, false);
	}
}

/**
* Обработчик ошибок парсера
*/
void AsyncXMLStream::onParseError(const char *message)
{
	onError(message);
}

/**
* Пир (peer) закрыл поток.
*
* Мы уже ничего не можем отправить в ответ,
* можем только корректно закрыть соединение с нашей стороны.
*/
void AsyncXMLStream::onPeerDown()
{
	fprintf(stderr, "#%d: [AsyncXMLStream: %d] peer down\n", getWorkerId(), fd);
	
	// читаем и парсим все, что осталось в потоке
	while ( 1 )
	{
		char buf[4096];
		ssize_t r = read(buf, sizeof(buf));
		if ( r <= 0 ) break;
		parseXML(buf, r, false);
	}
	
	// и сообщаем парсеру об EOF
	parseXML(0, 0, true);
}

/**
* Пир (peer) закрыл поток.
*
* Мы уже ничего не можем отправить в ответ,
* можем только корректно закрыть соединение с нашей стороны.
*/
void AsyncXMLStream::onShutdown()
{
	fprintf(stderr, "#%d: [AsyncXMLStream: %d] onShutdown\n", getWorkerId(), fd);
	
	// читаем и парсим все, что осталось в потоке
	while ( 1 )
	{
		char buf[4096];
		ssize_t r = read(buf, sizeof(buf));
		if ( r <= 0 ) break;
		parseXML(buf, r, false);
	}
	
	// и сообщаем парсеру об EOF
	parseXML(0, 0, true);
}
