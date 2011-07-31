#ifndef NETFLOW_SERVER_H
#define NETFLOW_SERVER_H

#include <nanosoft/netflow/netflowtypes.h>
#include <nanosoft/netflow/netflowcollector.h>
#include <nanosoft/core/udpserver.h>

#include <time.h>

inline unsigned int ip(unsigned char *addr)
{
	return addr[0] * 0x1000000 + addr[1] * 0x10000 + addr[2] * 0x100 + addr[3];
}

inline unsigned int ip(unsigned char a, unsigned char b, unsigned char c, unsigned char d)
{
	return a * 0x1000000 + b * 0x10000 + c * 0x100 + d;
}


inline unsigned char ip0(unsigned int addr)
{
	return addr / 0x1000000;
}

inline unsigned char ip1(unsigned int addr)
{
	return (addr / 0x10000) & 0xFF;
}

inline unsigned char ip2(unsigned int addr)
{
	return (addr / 0x100) & 0xFF;
}

inline unsigned char ip3(unsigned int addr)
{
	return addr & 0xFF;
}


struct nf_counter_t
{
	char name[32];
	unsigned int bytes;
	unsigned int packets;
	time_t time;
	nf_counter_t *next;
};

struct nf_filter_t
{
	unsigned int src_net;
	unsigned int src_mask;
	unsigned int dst_net;
	unsigned int dst_mask;
	nf_counter_t *counter;
	nf_filter_t *next;
};

/**
* Базовый класс для асинхронных серверов
*/
class NetFlowServer: public UDPServer
{
private:
	nf_counter_t *counters;
	nf_filter_t *filters;
	
protected:
	/**
	* Принять входящее соединение
	*/
	virtual void onRead();
	
	/**
	* Сигнал завершения работы
	*
	* Сервер решил закрыть соединение, здесь ещё есть время
	* корректно попрощаться с пиром (peer).
	*/
	virtual void onTerminate();
public:
	ptr<NetFlowCollector> collector;
	
	/**
	* Конструктор
	*/
	NetFlowServer();
	
	/**
	* Деструктор
	*/
	~NetFlowServer();
	
	/**
	* Найти счетчик, если его нет, то добавить и вернуть его
	*/
	nf_counter_t *lookupCounter(const char *name);
	
	/**
	* Найти фильтр
	*/
	nf_filter_t *lookupFilter(unsigned int src, unsigned int dst);
	
	/**
	* Загрузить конфигурацию из файла
	*/
	void configure(const char *filename);
};

#endif // NETFLOW_SERVER_H
