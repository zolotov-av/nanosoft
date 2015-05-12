
#include "pictl.h"

#include <avr/io.h>

typedef union
{
	char raw[32];
	struct pictl_packet packet;
} pictl_buffer;

struct pictl_state
{
	unsigned char offset;
	char state;
};

static pictl_buffer inp;
static struct pictl_state inp_state;
static pictl_on_packet_t callback_on_packet;

/**
* Инициализация pictl
*/
int pictl_init(pictl_on_packet_t on_packet)
{
	inp_state.offset = 0;
	inp_state.state = 0;
	callback_on_packet = on_packet;
}

/**
* Поместить полученный байт данных в pictl
*/
int pictl_put(char data)
{
	//PORTA = data;
	inp.raw[inp_state.offset++] = data;
	switch ( inp_state.state )
	{
	case 0:
		if ( inp.packet.len > 1 ) inp_state.state = 1;
		break;
	case 1:
		if ( inp.packet.len == inp_state.offset )
		{
			callback_on_packet(&inp.packet);
			inp_state.state = 0;
			inp_state.offset = 0;
		}
	}
}

/**
* Извлечь очередной байт данных для отправки
*/
char pictl_fetch()
{
	return 0x12;
}
