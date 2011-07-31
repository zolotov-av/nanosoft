#ifndef NETFLOW_TYPES_H
#define NETFLOW_TYPES_H

/**
* Заголовок пакета NetFlow v5
*/
struct nf_header_t
{
	unsigned short int version;
	unsigned short int count;
	unsigned int sys_uptime;
	unsigned int unix_secs;
	unsigned int unix_nsecs;
	unsigned int flow_sequence;
	unsigned char engine_type;
	unsigned char engine_id;
	unsigned short sampling_interval;
};

/**
* Flow-запись пакета NetFlow v5
*/
struct nf_record_t
{
	unsigned char srcaddr[4];
	unsigned char dstaddr[4];
	unsigned char nexthop[4];
	unsigned short input;
	unsigned short output;
	unsigned int packets;
	unsigned int octets;
	unsigned int first;
	unsigned int last;
	unsigned short srcport;
	unsigned short dstport;
	char pad1;
	unsigned char tcp_flags;
	unsigned char protocol;
	unsigned char tos;
	unsigned short src_as;
	unsigned short dst_as;
	unsigned char src_mask;
	unsigned char dst_mask;
	char pad2[2];
};

/**
* Пакет NetFlow v5
*/
struct nf_message_t
{
	nf_header_t header;
	nf_record_t records[30];
};

/**
* Выходной формат Flow-записи
*/
struct nf_flow_t
{
	unsigned int time;
	unsigned char srcaddr[4];
	unsigned char dstaddr[4];
	unsigned short srcport;
	unsigned short dstport;
	unsigned int octets;
	unsigned int packets;
	unsigned int userid;
	char classid[4];
};

#endif // NETFLOW_TYPES_H
