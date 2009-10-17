#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <iostream>
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
* Событие ошибки
*
* Вызывается в случае возникновения какой-либо ошибки.
* По умолчанию выводит все ошибки в stderr
*/
void AsyncXMLStream::onError(const char *message)
{
	cerr << "[AsyncXMLStream]: " << message << endl;
}

/**
* Обработчик ошибок парсера
*/
void AsyncXMLStream::onParseError(const char *message)
{
	onError(message);
}


/**
* Событие закрытия потока
*
* Вызывается в случае достижения конца файла
* или если противоположный конец закрыл поток
*/
void AsyncXMLStream::onShutdown()
{
	while ( 1 )
	{
		char buf[4096];
		ssize_t r = read(buf, sizeof(buf));
		if ( r <= 0 ) break;
		parseXML(buf, r, false);
	}
	parseXML(0, 0, true);
}
