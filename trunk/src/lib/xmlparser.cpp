#include <nanosoft/error.h>
#include <nanosoft/xmlparser.h>
#include <iostream>

using namespace std;

namespace nanosoft
{
	/**
	* Конструктор
	*/
	XMLParser::XMLParser(): parsing(false), resetNeed(false)
	{
		initParser();
	}
	
	/**
	* Деструктор
	*/
	XMLParser::~XMLParser()
	{
		XML_ParserFree(parser);
	}
	
	/**
	* Инициализация парсера
	*/
	void XMLParser::initParser()
	{
		parser = XML_ParserCreate((XML_Char *) "UTF-8");
		if ( parser == 0 ) error("[XMLParser] XML_ParserCreate() fault");
		XML_SetUserData(parser, (void*) this);
		XML_SetElementHandler(parser, startElementCallback, endElementCallback);
		XML_SetCharacterDataHandler(parser, characterDataCallback);
	}
	
	/**
	* Парсинг
	*/
	void XMLParser::parseXML(const char *buf, size_t len, bool isFinal)
	{
		cout << "\nparse: \033[22;31m" << string(buf, len) << "\033[0m\n";
		parsing = true;
		int r = XML_Parse(parser, buf, len, isFinal);
		parsing = false;
		if ( resetNeed ) realResetParser();
		else if ( ! r )
		{
			onParseError(XML_ErrorString(XML_GetErrorCode(parser)));
		}
	}
	
	/**
	* Реальная переинициализация парсера
	*/
	void XMLParser::realResetParser()
	{
		XML_ParserFree(parser);
		initParser();
	}
	
	/**
	* Сбросить парсер, начать парсить новый поток
	*/
	void XMLParser::resetParser()
	{
		if ( parsing ) resetNeed = true;
		else realResetParser();
	}
	
	/**
	* Обработчик открытия тега
	*/
	void XMLParser::startElementCallback(void *user_data, const XML_Char *name, const XML_Char **atts)
	{
		attributtes_t attributes;
		for(int i = 0; atts[i]; i += 2) {
			attributes[ atts[i] ] = atts[i + 1];
		}
		static_cast<XMLParser *>(user_data)->onStartElement(name, attributes);
	}
	
	/**
	* Отработчик символьных данных
	*/
	void XMLParser::characterDataCallback(void *user_data, const XML_Char *s, int len)
	{
		static_cast<XMLParser *>(user_data)->onCharacterData(string(s, len));
	}
	
	/**
	* Отбработчик закрытия тега
	*/
	void XMLParser::endElementCallback(void *user_data, const XML_Char *name)
	{
		static_cast<XMLParser *>(user_data)->onEndElement(name);
	}
}
