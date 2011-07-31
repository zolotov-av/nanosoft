#ifndef SNMP_PARSER_H
#define SNMP_PARSER_H

#include <nanosoft/snmp/asn1reader.h>

/**
* Парсер SNMP пакетов
*/
class SNMPParser: public ASN1Reader
{
protected:
	/**
	* Обработчик чтения переменной
	*/
	virtual void onGetValue(const int *oid, size_t len);
	
	/**
	* Обработчик чтения переменной
	*/
	virtual void onResponseValueInt(const int *oid, size_t len, int value);
	
	/**
	* GET-request oid binding
	*/
	bool parseGetValue(const unsigned char *&data, const unsigned char *limit);
	
	/**
	* GET-request oid bindings
	*/
	bool parseGetBindings(const unsigned char *&data, const unsigned char *limit);
	
	/**
	* GET-request
	*/
	bool parseGetRequest(const unsigned char *data, const unsigned char *limit);
	
	/**
	* GET-response oid binding
	*/
	bool parseResponseValue(const unsigned char *&data, const unsigned char *limit);
	
	/**
	* GET-response oid bindings
	*/
	bool parseResponseBindings(const unsigned char *&data, const unsigned char *limit);
	
	/**
	* GET-response
	*/
	bool parseResponse(const unsigned char *data, const unsigned char *limit);
public:
	/**
	* Версия пакета
	*/
	int version;
	
	/**
	* Community string
	*/
	char community[ASN1_STRING_MAXLEN+1];
	
	/**
	* ID запроса
	*/
	int requestID;
	
	int errorStatus;
	
	int errorIndex;
	
	/**
	* Конструктор
	*/
	SNMPParser();
	
	/**
	* Деструктор
	*/
	virtual ~SNMPParser();
	
	/**
	* Парсинг пакета
	*/
	bool parse(const unsigned char *data, const unsigned char *limit);
	
	/**
	* Парсинг пакета
	*/
	bool parse(const unsigned char *data, size_t size);
};

#endif // SNMP_PARSER_H
