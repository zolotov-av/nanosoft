#ifndef NANOSOFT_ASYNCXMLSTREAM_H
#define NANOSOFT_ASYNCXMLSTREAM_H

#include <string>
#include <nanosoft/asyncstream.h>
#include <nanosoft/xmlparser.h>

/**
* Асинхронный парсер XML потоков
*/
class AsyncXMLStream: public AsyncStream, public nanosoft::XMLParser
{
protected:
	/**
	* Событие готовности к чтению
	*
	* Вызывается когда в потоке есть данные,
	* которые можно прочитать без блокирования
	*/
	virtual void onRead();
	
	/**
	* Обработчик открытия тега
	*/
	virtual void onStartElement(const std::string &name, const attributtes_t &attributes) = 0;
	
	/**
	* Обработчик символьных данных
	*/
	virtual void onCharacterData(const std::string &cdata) = 0;
	
	/**
	* Обработчик закрытия тега
	*/
	virtual void onEndElement(const std::string &name) = 0;
	
	/**
	* Обработчик ошибок парсера
	*/
	virtual void onParseError(const char *message);
	
	/**
	* Пир (peer) закрыл поток.
	*
	* Мы уже ничего не можем отправить в ответ,
	* можем только корректно закрыть соединение с нашей стороны.
	*/
	virtual void onPeerDown();
	virtual void onShutdown();
public:
	/**
	* Класс описывающий атрибуты тега
	*/
	typedef std::map<std::string, std::string> attributtes_t;
	
	/**
	* Конструктор
	*/
	AsyncXMLStream(int afd);
	
	/**
	* Деструктор
	*/
	virtual ~AsyncXMLStream();
};

#endif // NANOSOFT_ASYNCXMLSTREAM_H
