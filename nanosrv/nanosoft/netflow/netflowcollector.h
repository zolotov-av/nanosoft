#ifndef NETFLOW_COLLECTOR_H
#define NETFLOW_COLLECTOR_H

#include <nanosoft/core/object.h>
#include <nanosoft/netflow/netflowtypes.h>

#include <time.h>
#include <stdio.h>

/**
* Описание подсети
*/
class NetFlowNetwork
{
public:
	unsigned int net;
	unsigned int mask;
	
	/**
	* Проверка принадлежит ли IP подсети
	*/
	bool contain(unsigned int ip)
	{
		return (ip & mask) == net;
	}
};

/**
* Описание класса трафика
*/
class NetFlowClass: public Object
{
public:
	char name[4];
	NetFlowNetwork src, dst;
	ptr<NetFlowClass> next;
};

/**
* Описание фильтра класса трафика
*/
class NetFlowClassFilter: public Object
{
public:
	NetFlowNetwork src, dst;
	ptr<NetFlowClass> pclass;
	ptr<NetFlowClassFilter> next;
};

/**
* Статистика
*/
struct NetFlowStat
{
	unsigned int rx_bytes;
	unsigned int rx_packets;
	unsigned int tx_bytes;
	unsigned int tx_packets;
};

/**
* Описание счетчика
*/
class NetFlowCounter: public Object
{
public:
	char name[16];
	FILE *f;
	NetFlowStat buffer[256];
	ptr<NetFlowCounter> next;
};

/**
* Описание фильтра счетчика
*/
class NetFlowCounterFilter: public Object
{
	NetFlowNetwork src, dst;
	ptr<NetFlowCounter> counter;
	ptr<NetFlowCounterFilter> next;
};

/**
* NetFlow коллектор
*/
class NetFlowCollector: public Object
{
private:
	ptr<NetFlowClass> classes;
	ptr<NetFlowClassFilter> classFilters;
	ptr<NetFlowCounter> counters;
	ptr<NetFlowCounterFilter> counterFilters;
	
	NetFlowClass *lookupClass(const char *name);
	NetFlowClassFilter *lookupClassFilter(unsigned int srcip, unsigned int dstip);
public:
	/**
	* Конструктор
	*/
	NetFlowCollector();
	
	/**
	* Деструктор
	*/
	~NetFlowCollector();
	
	/**
	* Добавить класс трафика
	*/
	bool addClass(const char *name);
	
	/**
	* Удалить класс трафика
	*/
	bool removeClass(const char *name);
	
	/**
	* Добавить фильтр класса трафика
	*/
	bool addClassFilter(const char *src, const char *dst, const char *classname);
	
	/**
	* Обновить фильтр класса трафика
	*/
	bool setClassFilter(const char *src, const char *dst, const char *classname);
	
	/**
	* Удалить фильтр класса трафика
	*/
	bool removeClassFilter(const char *src, const char *dst);
	
	/**
	* Добавить счетчик
	*/
	bool addCounter(const char *name);
	
	/**
	* Удалить счетчик
	*/
	bool removeCounter(const char *name);
	
	/**
	* Добавить фильтр счетчика
	*/
	int addCounterFilter(const char *src, const char *dst, const char *counter);
	
	/**
	* Обновить фильтр счетчика
	*/
	bool setCounterFilter(const char *src, const char *dst, const char *counter);
	
	/**
	* Удалить фильтр счетчика
	*/
	bool removeCounterFilter(const char *src, const char *dst);
	
	/**
	* Обработать Flow-запись
	*/
	void collect(time_t tm, const nf_record_t *flow);
	
	/**
	* Записать данные счетчиков на диск
	*/
	void commit();
	
	void flush();
};

#endif // NETFLOW_COLLECTOR_H
