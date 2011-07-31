
#include <nanosoft/netflow/netflowcollector.h>

#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

static nf_message_t message;

NetFlowCollector::NetFlowCollector()
{
	fprintf(stderr, "NetFlowCollector::NetFlowCollector()\n");
}

NetFlowCollector::~NetFlowCollector()
{
	fprintf(stderr, "NetFlowCollector::~NetFlowCollector()\n");
}

/**
* Добавить класс трафика
*/
bool NetFlowCollector::addClass(const char *name)
{
	ptr<NetFlowClass> c = new NetFlowClass;
	strncpy(c->name, name, 4);
	c->name[3] = 0;
	c->next = classes;
	classes = c;
}

bool parseNetwork(const char *net, unsigned int *addr, unsigned int *mask)
{
	int bytes[4], bits;
	int r = sscanf(net, "%d.%d.%d.%d/%d", &bytes[0], &bytes[1], &bytes[2], &bytes[3], &bits);
	if ( r == EOF ) return false;
	*addr = (bytes[0] & 0xFF) * 0x1000000 + (bytes[1] & 0xFF) * 0x10000 + (bytes[2] & 0xFF) * 0x100 + (bytes[3] & 0xFF);
	*mask = bits == 0 ? 0 : (0xFFFFFFFF << (32 - bits));
	return true;
}

NetFlowClass *NetFlowCollector::lookupClass(const char *name)
{
	NetFlowClass *c = classes.operator->();
	while ( c )
	{
		if ( strcmp(c->name, name) == 0 ) return c;
		c = c->next.operator->();
	}
	return 0;
}

NetFlowClassFilter *NetFlowCollector::lookupClassFilter(unsigned int srcip, unsigned int dstip)
{
	NetFlowClassFilter *f = classFilters.operator->();
	while ( f )
	{
		if ( f->src.contain(srcip) && f->dst.contain(dstip) ) return f;
		f = f->next.operator->();
	}
	return 0;
}

/**
* Добавить фильтр класса трафика
*/
bool NetFlowCollector::addClassFilter(const char *src, const char *dst, const char *classname)
{
	unsigned int srcnet;
	unsigned int srcmask;
	unsigned int dstnet;
	unsigned int dstmask;
	if ( ! parseNetwork(src, &srcnet, &srcmask) ) return false;
	if ( ! parseNetwork(dst, &dstnet, &dstmask) ) return false;
	NetFlowClass *c = lookupClass(classname);
	if ( c )
	{
		ptr<NetFlowClassFilter> filter = new NetFlowClassFilter;
		filter->src.net = srcnet;
		filter->src.mask = srcmask;
		filter->dst.net = dstnet;
		filter->dst.mask = dstmask;
		filter->pclass = c;
		filter->next = classFilters;
		classFilters = filter;
		return true;
	}
	return false;
}

void NetFlowCollector::collect(time_t tm, const nf_record_t *flow)
{
	nf_flow_t r;
	r.time = tm;
	memcpy(r.srcaddr, flow->srcaddr, 4);
	memcpy(r.dstaddr, flow->dstaddr, 4);
	r.srcport = flow->srcport;
	r.srcport = flow->dstport;
	r.octets = flow->octets;
	r.packets = flow->packets;
	r.userid = 0;
	NetFlowClassFilter *filter = lookupClassFilter(
		r.srcaddr[0] * 0xFFFFFF + r.srcaddr[1] * 0xFFFF + r.srcaddr[2] * 0xFF + r.srcaddr[3],
		r.dstaddr[0] * 0xFFFFFF + r.dstaddr[1] * 0xFFFF + r.dstaddr[2] * 0xFF + r.dstaddr[3]
		);
	strcpy(r.classid, filter ? filter->pclass->name : "unk");
	printf("%d.%d.%d.%d:%d -> %d.%d.%d.%d:%d [%s]\n",
		   r.srcaddr[0], r.srcaddr[1], r.srcaddr[2], r.srcaddr[3], r.srcport,
		   r.dstaddr[0], r.dstaddr[1], r.dstaddr[2], r.dstaddr[3], r.dstport,
		   r.classid);
}

void NetFlowCollector::flush()
{
}
