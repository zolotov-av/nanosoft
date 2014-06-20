
//#define F_CPU 1000000UL  // 1 MHz

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "pictl.h"

#define TIM2_OCI_ENABLE 1
#define TIM2_TOI_ENABLE 1

#include "timers.h"

#define DD_MISO PB6

volatile char counter = 1;

ISR(SPISTC_vect)
{
	pictl_put(SPDR);
	SPDR = pictl_fetch();
}

static unsigned char tim2_prescale = 0;
static unsigned int timer2 = 0;

ISR(TIMER2_COMP_vect)
{
	tim2_prescale++;
	if ( tim2_prescale == 10 )
	{
		tim2_prescale = 0;
		timer2++;
		PORTD = timer2;
	}
}

ISR(TIMER2_OVF_vect)
{
}

void command1(const struct pictl_packet *packet)
{
	if ( packet->len >= 4 )
	{
		unsigned char pin = packet->opt[0];
		if ( pin < 8 )
		{
			PORTA |= (1 << pin);
		}
	}
}

void command2(const struct pictl_packet *packet)
{
	if ( packet->len >= 4 )
	{
		unsigned char pin = packet->opt[0];
		if ( pin < 8 )
		{
			PORTA &= ~(1 << pin);
		}
	}
}

void pictl_on_packet(const struct pictl_packet *packet)
{
	switch (packet->cmd)
	{
	case 1:
		command1(packet);
		return;
	case 2:
		command2(packet);
		return;
	}
}

int main()
{
	timer2_init(TIMER_WGM_CTC, TIMER_CTC_NOPE, TIMER_CLOCK_1024);
	timer2_set_compare(97);
	
	// init SPI slave
	MISO_DDR = (1<<MISO_BIT);
	DDRA = 0xFF;
	DDRD = 0xFF;
	SPCR = (1 << SPIE) | (1 << SPE);
	PORTA = counter;
	PORTD = 1;
	SPDR = counter;
	pictl_init(pictl_on_packet);

	while ( 1 )
	{
		//PORTA = spi_read(counter++);
		sei();
		set_sleep_mode(0);
		sleep_enable();
		sleep_cpu();
	}
	
	return 0;
}
