#ifndef NANOSOFT_ASYNCXMLSTREAM_H
#define NANOSOFT_ASYNCXMLSTREAM_H

#include <expat.h>
#include <map>
#include <string>
#include <nanosoft/asyncstream.h>

/**
* Асинхронный парсер XML потоков
*/
class AsyncXMLStream: public AsyncStream
{
private:
	/**
	* Парсер expat
	*/
	XML_Parser parser;
	
	/**
	* Признак парсинга
	* TRUE - парсер в состоянии обработка куска файла
	*/
	bool parsing;
	
	/**
	* Признак необходимости перенинициализации парсера
	* TRUE - парсер должен быть переинициализован перед
	*   обработкой следующего куска файла
	*/
	bool resetNeed;
	
	/**
	* Инициализация парсера
	*/
	void initParser();
	
	/**
	* Парсинг
	*/
	void parse(const char *buf, size_t len, bool isFinal);
	
	/**
	* Реальная переинициализация парсера
	*/
	void realResetParser();
	
	/**
	* Обработчик открытия тега
	*/
	static void startElementCallback(void *user_data, const XML_Char *name, const XML_Char **atts);
	
	/**
	* Отработчик символьных данных
	*/
	static void characterDataCallback(void *user_data, const XML_Char *s, int len);
	
	/**
	* Отбработчик закрытия тега
	*/
	static void endElementCallback(void *user_data, const XML_Char *name);
	
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
	
	/**
	* Сбросить парсер, начать парсить новый поток
	*/
	void resetParser();
	
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
