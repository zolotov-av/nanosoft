
//#define F_CPU 1000000UL  // 1 MHz

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "pictl.h"
#include "iom16a_usart.h"

#define TIM2_OCI_ENABLE 1
#define TIM2_TOI_ENABLE 1

#include "timers.h"

#define DD_MISO PB6
/*
unsigned char counter = 1;
char counter_enable = 1;
char x = 0;
*/
/*
ISR(SPISTC_vect)
{
	pictl_put(SPDR);
	SPDR = pictl_fetch();
}
*/

/*
static unsigned char tim2_prescale = 0;
static unsigned int timer2 = 0;

ISR(TIMER2_COMP_vect)
{
	tim2_prescale++;
	if ( tim2_prescale == 10 )
	{
		tim2_prescale = 0;
		timer2++;
		if ( counter_enable )
		{
			//PORTC = ++counter;
		}
	}
}
*/
//ISR(TIMER2_OVF_vect)
//{
//}
/*
void cmd_set_enabled(const struct pictl_packet *packet)
{
	if ( packet->len >= 4 )
	{
		counter_enable = packet->opt[0];
	}
}

void cmd_set_counter(const struct pictl_packet *packet)
{
	if ( packet->len >= 4 )
	{
		//PORTC = counter = packet->opt[0];
	}
}

void pictl_on_packet(const struct pictl_packet *packet)
{
	switch (packet->cmd)
	{
	case 1:
		cmd_set_enabled(packet);
		return;
	case 2:
		cmd_set_counter(packet);
		return;
	}
}
*/
#define UART_BAUD_K 207

char spi_data;

int main()
{
	//timer2_init(TIMER_WGM_CTC, TIMER_CTC_NOPE, TIMER_CLOCK_1024);
	//timer2_set_compare(97);
	
	// init SPI slave
	MISO_DDR = (1<<MISO_BIT);
	DDRA = 0xFF;
	DDRC = 0xFF;
	SPCR = (1 << SPIE) | (1 << SPE);
	PORTA = 0xFF;
	PORTC = 0xFF;
	//pictl_init(pictl_on_packet);
	
	
	UCSRA = 0;
	UCSRB = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE)|(1<<TXCIE)|(1<<UDRIE);
	UCSRC = (1<<URSEL)|(1<<UCSZ0)|(1<<UCSZ1);
	
	UBRRH = (UART_BAUD_K / 256) & ~(1 <<URSEL);
	UBRRL = UART_BAUD_K % 256;
	
	
	spi_init();
	
	while ( 1 )
	{
		sei();
		set_sleep_mode(0);
		sleep_enable();
		sleep_cpu();
		
		
		if ( usart_getc(&spi_data) )
		{
			usart_putc(spi_data);
		}
		
		if ( spi_getc(&spi_data) )
		{
			spi_putc(spi_data + 100);
		}
	}
	
	return 0;
}
