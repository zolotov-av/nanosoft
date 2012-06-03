#ifndef NANOSOFT_XMLPARSER_H
#define NANOSOFT_XMLPARSER_H

#include <expat.h>
#include <map>
#include <string>
#include "zlib.h"

namespace nanosoft
{
	/**
	* XML парсер
	*/
	class XMLParser
	{
	public:
		/**
		* Класс описывающий атрибуты тега
		*/
		typedef std::map<std::string, std::string> attributtes_t;
		
		/**
		* Конструктор
		*/
		XMLParser();
		
		/**
		* Деструктор
		*/
		virtual ~XMLParser();
		
		/**
		* Вернуть флаг компрессии
		* @return TRUE - компрессия включена, FALSE - компрессия отключена
		*/
		bool getCompression();
		
		/**
		* Включить/отключить компрессию
		* @param state TRUE - включить компрессию, FALSE - отключить компрессию
		* @return TRUE - операция успешна, FALSE - операция прошла с ошибкой
		*/
		bool setCompression(bool state);
		
		/**
		* Парсинг XML
		* 
		* Если включена компрессия, то данные сначала распаковываются
		* 
		* @param data буфер с данными
		* @param len длина буфера с данными
		* @param isFinal TRUE - последний кусок, FALSE - будет продолжение
		* @return TRUE - успешно, FALSE - ошибка парсинга
		*/
		bool parseXML(const char *data, size_t len, bool isFinal);
		
		/**
		* Сбросить парсер, начать парсить новый поток
		*/
		void resetParser();
		
	protected:
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
		virtual void onParseError(const char *message) = 0;
		
	private:
		/**
		* Парсер expat
		*/
		XML_Parser parser;
		
		/**
		* Контекст компрессора zlib
		*/
		z_stream strm;
		
		/**
		* Флаг компрессии zlib
		*
		* TRUE - компрессия включена
		* FALSE - компрессия отключена
		*/
		bool compression;
		
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
		* Включить компрессию
		*/
		bool enableCompression();
		
		/**
		* Отключить компрессию
		*/
		bool disableCompression();
		
		/**
		* Инициализация парсера
		*/
		bool initParser();
		
		/**
		* Реальная переинициализация парсера
		*/
		bool realResetParser();
		
		/**
		* Парсинг XML
		*
		* @param buf буфер с данными
		* @param len длина буфера с данными
		* @param isFinal TRUE - последний кусок, FALSE - будет продолжение
		* @return TRUE - успешно, FALSE - ошибка парсинга
		*/
		bool realParseXML(const char *buf, size_t len, bool isFinal);
		
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
	};
}

#endif // NANOSOFT_XMLPARSER_H
