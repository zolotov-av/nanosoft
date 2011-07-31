
#include <nanosoft/netflow/netflowserver.h>

#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

static nf_message_t message;

NetFlowServer::NetFlowServer(): counters(0), filters(0)
{
	collector = new NetFlowCollector;
}

NetFlowServer::~NetFlowServer()
{
}

/**
* Найти счетчик, если его нет, то добавить и вернуть его
*/
nf_counter_t * NetFlowServer::lookupCounter(const char *name)
{
	nf_counter_t *p = counters;
	while ( p )
	{
		if ( strcmp(p->name, name) == 0 ) return p;
		p = p->next;
	}
	p = new nf_counter_t;
	strcpy(p->name, name);
	p->bytes = 0;
	p->packets = 0;
	p->time = 0;
	p->next = counters;
	counters = p;
	return p;
}

/**
* Найти фильтр
*/
nf_filter_t *NetFlowServer::lookupFilter(unsigned int src, unsigned int dst)
{
	nf_filter_t *p = filters;
	while ( p )
	{
		if ( (src & p->src_mask) == p->src_net && (dst & p->dst_mask) == p->dst_net ) return p;
		p = p->next;
	}
	return 0;
}

/**
* Загрузить конфигурацию из файла
*/
void NetFlowServer::configure(const char *filename)
{
	FILE *f = fopen(filename, "r");
	int r;
	char cname[32], act[32];
	int src[4], srcbits;
	int dst[4], dstbits;
	while (1)
	{
		int r = fscanf(f, "%s %s %d.%d.%d.%d/%d %d.%d.%d.%d/%d", cname, act,
			&src[0], &src[1], &src[2], &src[3], &srcbits,
			&dst[0], &dst[1], &dst[2], &dst[3], &dstbits);
		/*printf("%d [%s] add %d.%d.%d.%d/%d %d.%d.%d.%d/%d\n", r, cname,
			src[0], src[1], src[2], src[3], srcbits,
			dst[0], dst[1], dst[2], dst[3], dstbits);*/
		if ( r == EOF ) break;
		if ( r != 12 )
		{
			fprintf(stderr, "wrong line\n");
			continue;
		}
		printf("add [%s] from %d.%d.%d.%d/%d to %d.%d.%d.%d/%d\n", cname,
			src[0], src[1], src[2], src[3], srcbits,
			dst[0], dst[1], dst[2], dst[3], dstbits);
		nf_filter_t *filter = new nf_filter_t;
		filter->src_mask = srcbits == 0 ? 0 : (0xFFFFFFFF << (32 - srcbits));
		filter->src_net = ip(src[0], src[1], src[2], src[3]) & filter->src_mask;
		filter->dst_mask = dstbits == 0 ? 0 : (0xFFFFFFFF << (32 - dstbits));
		filter->dst_net = ip(dst[0], dst[1], dst[2], dst[3]) & filter->dst_mask;
		filter->counter = lookupCounter(cname);
		filter->next = filters;
		filters = filter;
	}
	fclose(f);
}

/**
* Принять входящее соединение
*/
void NetFlowServer::onRead()
{
	socklen_t len;
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	int status = recvfrom(fd, &message, sizeof(message), 0, (struct sockaddr*)&addr, &len);
	if ( status == -1 )
	{
		stderror();
		return;
	}
	unsigned int mask = ip(255, 255, 0, 0);
	unsigned int net = ip(192, 168, 6, 11) & mask;
	
	if ( ntohs(message.header.version) == 5 )
	{
		nf_record_t *nfr = message.records;
		time_t tm = htonl(message.header.unix_secs);
		for(int i = 0; i < ntohs(message.header.count); i++, nfr++)
		{
			unsigned int src = ip(nfr->srcaddr);
			unsigned int dst = ip(nfr->dstaddr);
			bool test = (src & mask) == net;
			nf_filter_t *filter = lookupFilter(src, dst);
			printf("%d.%d.%d.%d:%d -> %d.%d.%d.%d:%d proto: %d, packets: %u, octets: %u, counter: %s\n",
				ip0(src), ip1(src), ip2(src), ip3(src), ntohs(nfr->srcport),
				ip0(dst), ip1(dst), ip2(dst), ip3(dst), ntohs(nfr->dstport),
				nfr->protocol, ntohl(nfr->packets), ntohl(nfr->octets), (filter ? filter->counter->name : "<no>"));
			collector->collect(tm, nfr);
		}
	}
}

/**
* Сигнал завершения работы
*
* Сервер решил закрыть соединение, здесь ещё есть время
* корректно попрощаться с пиром (peer).
*/
void NetFlowServer::onTerminate()
{
	fprintf(stderr, "NetFlowServer::onTerminate()\n");
}
