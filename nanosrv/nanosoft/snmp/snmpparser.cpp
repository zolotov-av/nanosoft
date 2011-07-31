
#include <nanosoft/snmp/snmpparser.h>

#include <stdio.h>
#include <string.h>

SNMPParser::SNMPParser()
{
}

SNMPParser::~SNMPParser()
{
}

/**
* Обработчик чтения переменной
*/
void SNMPParser::onGetValue(const int *oid, size_t len)
{
	printf("          GET(%d.%d", oid[0], oid[1]);
	for(size_t i = 2; i < len; i++) printf(".%d", oid[i]);
	printf(")\n");
}

/**
* GET-request oid binding
*/
bool SNMPParser::parseGetValue(const unsigned char *&data, const unsigned char *limit)
{
	printf("        GET value\n");
	
	asn1_header_t h;
	if ( ! readHeader(h, data, limit) ) return false;
	
	const unsigned char *p = h.data;
	const unsigned char *end = h.limit;
	
	int oid[ASN1_OID_MAXLEN];
	size_t len;
	if ( ! readOID(oid, len, p, end) ) return false;
	printf("          OID: %d.%d", oid[0], oid[1]);
	for(size_t i = 2; i < len ; i++)
	{
		printf(".%d", oid[i]);
	}
	printf("\n");
	
	if ( ! readNULL(p, end) ) return false;
	
	onGetValue(oid, len);
	
	data = end;
	
	return true;
}

/**
* GET-request oid bindings
*/
bool SNMPParser::parseGetBindings(const unsigned char *&data, const unsigned char *limit)
{
	printf("      GET bindings, 0x%02X\n", data[0]);
	
	asn1_header_t h;
	if ( ! readHeader(h, data, limit) ) return false;
	
	const unsigned char *p = h.data;
	const unsigned char *end = h.limit;
	
	while ( p < end )
	{
		if ( ! parseGetValue(p, end) ) return false;
	}
	
	data = end;
	
	return true;
}

/**
* GET-request
*/
bool SNMPParser::parseGetRequest(const unsigned char *data, const unsigned char *limit)
{
	printf("  parse SNMP GET-request\n");
	
	if ( ! readInt(requestID, data, limit) ) return false;
	printf("    requestID: %d\n", requestID);
	
	if ( ! readInt(errorStatus, data, limit) ) return false;
	printf("    errorStatus: %d\n", errorStatus);
	
	if ( ! readInt(errorIndex, data, limit) ) return false;
	printf("    errorIndex: %d\n", errorIndex);
	
	if ( ! parseGetBindings(data, limit) ) return false;
	
	return true;
}

/**
* Обработчик чтения переменной
*/
void SNMPParser::onResponseValueInt(const int *oid, size_t len, int value)
{
	printf("          GET(%d.%d", oid[0], oid[1]);
	for(size_t i = 2; i < len; i++) printf(".%d", oid[i]);
	printf(") = %u\n", value);
}

/**
* GET-response oid binding
*/
bool SNMPParser::parseResponseValue(const unsigned char *&data, const unsigned char *limit)
{
	printf("        Response value\n");
	
	asn1_header_t h;
	if ( ! readHeader(h, data, limit) ) return false;
	if ( h.type != 0x30 ) return false;
	
	const unsigned char *p = h.data;
	const unsigned char *end = h.limit;
	
	int oid[ASN1_OID_MAXLEN];
	size_t len;
	if ( ! readOID(oid, len, p, end) ) return false;
	printf("          OID: %d.%d", oid[0], oid[1]);
	for(size_t i = 2; i < len ; i++)
	{
		printf(".%d", oid[i]);
	}
	printf("\n");
	
	int value;
	if ( ! readInt(value, p, end) ) return false;
	
	onResponseValueInt(oid, len, value);
	
	data = end;
	
	return true;
}

/**
* GET-response oid bindings
*/
bool SNMPParser::parseResponseBindings(const unsigned char *&data, const unsigned char *limit)
{
	printf("      Response bindings, 0x%02X\n", data[0]);
	
	asn1_header_t h;
	if ( ! readHeader(h, data, limit) ) return false;
	if ( h.type != 0x30 ) return false;
	
	const unsigned char *p = h.data;
	const unsigned char *end = h.limit;
	
	while ( p < end )
	{
		if ( ! parseResponseValue(p, end) ) return false;
	}
	
	data = end;
	
	return true;
}

/**
* GET-response
*/
bool SNMPParser::parseResponse(const unsigned char *data, const unsigned char *limit)
{
	printf("  parse SNMP GET-response\n");
	
	if ( ! readInt(requestID, data, limit) ) return false;
	printf("    requestID: %d\n", requestID);
	
	if ( ! readInt(errorStatus, data, limit) ) return false;
	printf("    errorStatus: %d\n", errorStatus);
	
	if ( ! readInt(errorIndex, data, limit) ) return false;
	printf("    errorIndex: %d\n", errorIndex);
	
	if ( ! parseResponseBindings(data, limit) ) return false;
	
	return true;
}

/**
* Парсинг пакета
*/
bool SNMPParser::parse(const unsigned char *data, const unsigned char *limit)
{
	printf("parse SNMP packet\n");
	asn1_header_t h;
	if ( ! readHeader(h, data, limit) ) return false;
	printf("SNMP packet, type: 0x%02X\n", h.type);
	
	if ( ! readInt(version, data, limit) ) return false;
	printf("SNMP version: %d\n", version);
	if ( version != 1 ) return false;
	
	if ( ! readString(community, data, limit) ) return false;
	printf("SNMP community: '%s'\n", community);
	
	if ( ! readHeader(h, data, limit) ) return false;
	printf("SNMP sub-packet, type: 0x%02X\n", h.type);
	switch ( h.type )
	{
	case 0xA0:
		if ( ! parseGetRequest(data, limit) ) return false;
		break;
	case 0xA2:
		if ( ! parseResponse(data, limit) ) return false;
		break;
	default:
		printf("unknown SNMP command: 0x%02X\n", h.type);
		return false;
	}
	
	return true;
}

/**
* Парсинг пакета
*/
bool SNMPParser::parse(const unsigned char *data, size_t size)
{
	bool status = parse(data, data + size);
	printf("%s", status ? "parse ok\n" : "parse fail\n");
	return status;
}
