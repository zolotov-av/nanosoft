
#include <nanosoft/snmp/snmpserver.h>

#include <stdio.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

SNMPServer::SNMPServer(): SNMPBuilder(65536)
{
	deep = 0;
	prefix[0] = 0;
}

SNMPServer::~SNMPServer()
{
}

void SNMPServer::onNull(const char *data, const char *limit)
{
	printf("%sNULL[%d]\n", prefix, limit - data);
}

void SNMPServer::onOID(const char *data, const char *limit)
{
	printf("%sOID[%d]: ", prefix, limit - data);
	const char *p = data;
	int item = 0;
	while ( p < limit )
	{
		unsigned char c = (unsigned char)p[0];
		unsigned char d = c & 0x7F;
		item = item * 128 + d;
		p++;
		if ( (c & 0x80) == 0 ) break;
	}
	printf("%d.%d", item / 40, item % 40);
	while ( p < limit )
	{
		item = 0;
		while ( p < limit )
		{
			unsigned char c = (unsigned char)p[0];
			unsigned char d = c & 0x7F;
			item = item * 128 + d;
			p++;
			if ( (c & 0x80) == 0 ) break;
		}
		printf(".%d", item);
	}
	printf("\n");
}

void SNMPServer::onString(const char *data, const char *limit)
{
	char buf[4096];
	int len = limit - data;
	for(int i = 0; i < len; i++) buf[i] = data[i];
	buf[len] = 0;
	printf("%sSTRING: %s\n", prefix, buf);
}

void SNMPServer::onInteger(const char *data, const char *limit)
{
	unsigned int value = 0;
	while ( data < limit )
	{
		value = value * 256 + (unsigned char)data[0];
		data++;
	}
	printf("%sINTEGER: %d\n", prefix, value);
}

void SNMPServer::onSNMPGet(const char *data, const char *limit)
{
	printf("%sSNMP GET\n", prefix);
	while ( data < limit )
	{
		onASN(data[0], data+2, data+2+data[1]);
		data += 2 + data[1];
	}
}

void SNMPServer::onSNMP(const char *data, const char *limit)
{
	printf("%sSNMP message, len: %d\n", prefix, limit - data);
	while ( data < limit )
	{
		onASN(data[0], data+2, data+2+data[1]);
		data += 2 + data[1];
	}
}

/**
* Парсер ASN.1
*/
void SNMPServer::onASN(char type, const char *data, const char *limit)
{
	//printf("ASN.1, type: 0x%x, len: %d\n", (unsigned char)type, limit - data);
	prefix[deep*2] = ' ';
	prefix[deep*2+1] = ' ';
	prefix[deep*2+2] = 0;
	deep++;
	switch ( (unsigned char)type )
	{
	case 0x02:
		onInteger(data, limit);
		break;
	case 0x04:
		onString(data, limit);
		break;
	case 0x05:
		onNull(data, limit);
		break;
	case 0x06:
		onOID(data, limit);
		break;
	case 0x30:
		onSNMP(data, limit);
		break;
	case 0xa0:
		onSNMPGet(data, limit);
		break;
	default:
		printf("%sunknown tag(0x%X), skip it\n", prefix, (unsigned char)type);
	}
	deep--;
	prefix[deep*2] = 0;
}

/**
* Парсер SNMP PDU value bindings
*/
bool SNMPServer::parseSNMPValues(const unsigned char *&data, const unsigned char *limit)
{
	if ( (data + 1) >= limit ) return false;
	if ( data[0] != 0xA0 ) return false;
	if ( data + data[1] > limit ) return false;
	data += 2;
	
	while ( data < limit )
	{
		data ++;
	}
}

/**
* Парсер SNMP PDU GET
*/
bool SNMPServer::parseSNMPGet(const unsigned char *&data, const unsigned char *limit)
{
	if ( (data + 1) >= limit ) return false;
	if ( data[0] != 0xA0 ) return false;
	if ( data + data[1] > limit ) return false;
	data += 2;
	
	int requestID, errorStatus, errorIndex;
	
	if ( ! readInt(requestID, data, limit) ) return false;
	printf("SNMP requestID: %d\n", requestID);
	
	if ( ! readInt(errorStatus, data, limit) ) return false;
	printf("SNMP errorStatus: %d\n", errorStatus);
	
	if ( ! readInt(errorIndex, data, limit) ) return false;
	printf("SNMP errorIndex: %d\n", errorIndex);
	
	if ( ! parseSNMPValues(data, limit) ) return false;

	return true;
}

/**
* Парсер SNMP-пакета
*/
bool SNMPServer::parseSNMP(const unsigned char *data, const unsigned char *limit)
{
	printf("parse SNMP packet\n");
	if ( (data + 1) >= limit ) return false;
	if ( data[0] != 0x30 ) return false;
	if ( data + data[1] > limit ) return false;
	data += 2;
	
	int version;
	if ( ! readInt(version, data, limit) ) return false;
	printf("SNMP version: %d\n", version);
	if ( version != 1 ) return false;
	
	char community[256];
	if ( ! readString(community, data, limit) ) return false;
	printf("SNMP community: '%s'\n", community);
	
	switch ( data[0] )
	{
	case 0xA0:
		if ( ! parseSNMPGet(data, limit) ) return false;
		break;
	default:
		printf("unknown SNMP command\n");
	}
	
	printf("parse ok\n");
}

/**
* Принять входящее соединение
*/
void SNMPServer::onRead()
{
	//socklen_t len;
	//struct sockaddr_storage addr;
	//memset(&addr, 0, sizeof(struct sockaddr_storage));
	unsigned char message[4096];
	int status = recv(fd, message, sizeof(message), 0);
	if ( status >= 0 )
	{
		message[status] = 0;
		printf("snmp message[len: %d, type: %02X, len2: %d]\n", status, message[0], message[1]);
		//onASN(message[0], (const char *)message + 2, (const char *)message + 2 + message[1]);
		if ( parse((unsigned char *)message, status) )
		{
			//send("192.169.13.1", message, status);
		}
	}
	else
	{
		stderror();
		printf("recvfrom fail, status: %d\n", status);
		return;
	}
}

void SNMPServer::send(const char *host, const unsigned char *message, size_t len)
{
	struct addrinfo hints, *servinfo, *p;
	int rv;
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	
	if ((rv = getaddrinfo(host, "161", &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return;
	}
	
	int status = sendto(fd, message, len, 0, servinfo->ai_addr, servinfo->ai_addrlen);
	if ( status == -1 ) stderror();
	
	freeaddrinfo(servinfo);
}

/**
* Отправить GET-запрос
*/
void SNMPServer::sendGET(const char *host, const char *community, const int *oid, size_t len)
{
	reset(community);
	setCommand(0xA0);
	setRequestID(time(0));
	addNull(oid, len);
	commit();
	send(host, getData(), getDataSize());
}

void SNMPServer::des3028Stat(const char *host, const char *community)
{
	reset(community);
	setCommand(0xA0);
	setRequestID(time(0));
	static int in_octets[] = {1,3,6,1,2,1,2,2,1,10,0};
	static int out_octets[] = {1,3,6,1,2,1,2,2,1,16,0};
	static int in_packets[] = {1,3,6,1,2,1,2,2,1,11,0};
	static int out_packets[] = {1,3,6,1,2,1,2,2,1,17,0};
	const int in_octets_len = sizeof(in_octets) / sizeof(in_octets[0]);
	const int out_octets_len = sizeof(out_octets) / sizeof(out_octets[0]);
	const int in_packets_len = sizeof(in_packets) / sizeof(in_packets[0]);
	const int out_packets_len = sizeof(out_packets) / sizeof(out_packets[0]);
	//static int oid[] = {.1.3.6.1.2.1.2.2.1.16.2};
	
	for(int i = 1; i <= 27; i++)
	{
		in_octets[in_octets_len-1] = i;
		out_octets[out_octets_len-1] = i;
		in_packets[in_packets_len-1] = i;
		out_packets[out_packets_len-1] = i;
		addNull(in_octets, in_octets_len);
		addNull(out_octets, out_octets_len);
		//addNull(in_packets, in_packets_len);
		//addNull(out_packets, out_packets_len);
	}
	commit();
	printf("Packet size: %d\n", getDataSize());
	send(host, getData(), getDataSize());
}

void SNMPServer::des3028Stat2(const char *host, const char *community)
{
	reset(community);
	setCommand(0xA0);
	setRequestID(time(0));
	static int in_octets[] = {1,3,6,1,2,1,2,2,1,10,0};
	static int out_octets[] = {1,3,6,1,2,1,2,2,1,16,0};
	static int in_packets[] = {1,3,6,1,2,1,2,2,1,11,0};
	static int out_packets[] = {1,3,6,1,2,1,2,2,1,17,0};
	const int in_octets_len = sizeof(in_octets) / sizeof(in_octets[0]);
	const int out_octets_len = sizeof(out_octets) / sizeof(out_octets[0]);
	const int in_packets_len = sizeof(in_packets) / sizeof(in_packets[0]);
	const int out_packets_len = sizeof(out_packets) / sizeof(out_packets[0]);
	//static int oid[] = {.1.3.6.1.2.1.2.2.1.16.2};
	
	for(int i = 1; i <= 27; i++)
	{
		in_octets[in_octets_len-1] = i;
		out_octets[out_octets_len-1] = i;
		in_packets[in_packets_len-1] = i;
		out_packets[out_packets_len-1] = i;
		//addNull(in_octets, in_octets_len);
		//addNull(out_octets, out_octets_len);
		addNull(in_packets, in_packets_len);
		addNull(out_packets, out_packets_len);
	}
	commit();
	printf("Packet size: %d\n", getDataSize());
	send(host, getData(), getDataSize());
}

/**
* Сигнал завершения работы
*
* Сервер решил закрыть соединение, здесь ещё есть время
* корректно попрощаться с пиром (peer).
*/
void SNMPServer::onTerminate()
{
	fprintf(stderr, "SNMPServer::onTerminate()\n");
}
