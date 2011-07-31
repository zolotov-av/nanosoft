#ifndef SNMP_SERVER_H
#define SNMP_SERVER_H

#include <nanosoft/snmp/snmpparser.h>
#include <nanosoft/snmp/snmpbuilder.h>
#include <nanosoft/core/udpserver.h>

/**
* Сервер SNMP
*/
class SNMPServer: public UDPServer, public SNMPParser, public SNMPBuilder
{
protected:
	int deep;
	char prefix[80];
	
	/**
	* Принять входящее соединение
	*/
	virtual void onRead();
	
	void onNull(const char *data, const char *limit);
	
	void onOID(const char *data, const char *limit);
	
	void onString(const char *data, const char *limit);
	
	void onInteger(const char *data, const char *limit);
	
	void onSNMPGet(const char *data, const char *limit);
	
	void onSNMP(const char *data, const char *limit);
	
	/**
	* Парсер ASN.1
	*/
	void onASN(char type, const char *data, const char *limit);
	
	/**
	* Парсер SNMP PDU value bindings
	*/
	bool parseSNMPValues(const unsigned char *&data, const unsigned char *limit);
	
	/**
	* Парсер SNMP PDU GET
	*/
	bool parseSNMPGet(const unsigned char *&data, const unsigned char *limit);
	
	/**
	* Парсер SNMP-пакета
	*/
	bool parseSNMP(const unsigned char *data, const unsigned char *limit);
	
	/**
	* Сигнал завершения работы
	*
	* Сервер решил закрыть соединение, здесь ещё есть время
	* корректно попрощаться с пиром (peer).
	*/
	virtual void onTerminate();
public:
	/**
	* Конструктор
	*/
	SNMPServer();
	
	/**
	* Деструктор
	*/
	~SNMPServer();
	
	/**
	* Отправить GET-запрос
	*/
	void sendGET(const char *host, const char *community, const int *oid, size_t len);
	
	void send(const char *host, const unsigned char *message, size_t len);
	
	void des3028Stat(const char *host, const char *community);
	void des3028Stat2(const char *host, const char *community);
};

#endif // SNMP_SERVER_H
