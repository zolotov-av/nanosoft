#ifndef __PICTL_H_
#define __PICTL_H_

struct pictl_packet
{
	unsigned char len;
	unsigned char cmd;
	unsigned char pid;
	unsigned char opt[1];
};

typedef void (*pictl_on_packet_t)(const struct pictl_packet *packet);

/**
* Инициализация pictl
*/
int pictl_init(pictl_on_packet_t on_packet);

/**
* Поместить полученный байт данных в pictl
*/
int pictl_put(char data);

/**
* Извлечь очередной байт данных для отправки
*/
char pictl_fetch();

#endif // __PICTL_H_
