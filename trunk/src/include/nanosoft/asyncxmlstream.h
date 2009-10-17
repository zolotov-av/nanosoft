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
	
protected:
	/**
	* Событие готовности к чтению
	*
	* Вызывается когда в потоке есть данные,
	* которые можно прочитать без блокирования
	*/
	virtual void onRead();
	
	/**
	* Событие готовности к записи
	*
	* Вызывается, когда в поток готов принять
	* данные для записи без блокировки
	*/
	virtual void onWrite() = 0;
	
	/**
	* Событие ошибки
	*
	* Вызывается в случае возникновения какой-либо ошибки
	*/
	virtual void onError(const char *message);
	
	/**
	* Обработчик ошибок парсера
	*/
	virtual void onParseError(const char *message);
	
	/**
	* Событие закрытия потока
	*
	* Вызывается в случае достижения конца файла
	* или если противоположный конец закрыл поток
	*/
	virtual void onShutdown();
	
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
};

#endif // NANOSOFT_ASYNCXMLSTREAM_H
