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
	parser = XML_ParserCreate((XML_Char *) "UTF-8");
	XML_SetUserData(parser, (void*) this);
	XML_SetElementHandler(parser, startElementCallback, endElementCallback);
	XML_SetCharacterDataHandler(parser, characterDataCallback);
}

/**
* Деструктор
*/
AsyncXMLStream::~AsyncXMLStream()
{
	XML_ParserFree(parser);
}

/**
* Сбросить парсер, начать парсить новый поток
*/
void AsyncXMLStream::resetParser()
{
	XML_ParserFree(parser);
	parser = XML_ParserCreate((XML_Char *) "UTF-8");
	XML_SetUserData(parser, (void*) this);
	XML_SetElementHandler(parser, startElementCallback, endElementCallback);
	XML_SetCharacterDataHandler(parser, characterDataCallback);
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
		cout << "parse: " << string(buf, r);
		if ( ! XML_Parse(parser, buf, r, false) )
		{
			onError(XML_ErrorString(XML_GetErrorCode(parser)));
		}
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
		if ( ! XML_Parse(parser, buf, r, false) )
		{
			onError(XML_ErrorString(XML_GetErrorCode(parser)));
		}
	}
	
	if ( ! XML_Parse(parser, 0, 0, true) )
	{
		onError(XML_ErrorString(XML_GetErrorCode(parser)));
	}
}



/**
* Обработчик открытия тега
*/
void AsyncXMLStream::startElementCallback(void *user_data, const XML_Char *name, const XML_Char **atts)
{
	attributtes_t attributes;
	for(int i = 0; atts[i]; i += 2) {
		attributes[ atts[i] ] = atts[i + 1];
	}
	static_cast<AsyncXMLStream *>(user_data)->onStartElement(name, attributes);
}

/**
* Отработчик символьных данных
*/
void AsyncXMLStream::characterDataCallback(void *user_data, const XML_Char *s, int len)
{
	static_cast<AsyncXMLStream *>(user_data)->onCharacterData(string(s, len));
}

/**
* Отбработчик закрытия тега
*/
void AsyncXMLStream::endElementCallback(void *user_data, const XML_Char *name)
{
	static_cast<AsyncXMLStream *>(user_data)->onEndElement(name);
}
