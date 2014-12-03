
//#define F_CPU 1000000UL  // 1 MHz

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define TIM2_OCI_ENABLE 1
#define TIM2_TOI_ENABLE 1

#include "iot13_timer.h"

static prescaler_cnt = 0;

ISR(TIM0_COMPA_vect)
{
	if ( prescaler_cnt < 6 )
	{
		PORTB ^= (1<<PB3);
	}
	else
	{
		PORTB ^= (1<<PB4);
	}
	prescaler_cnt++;
	if (prescaler_cnt==12) prescaler_cnt = 0;
}

int main()
{
//	timer2_init(TIMER_WGM_CTC, TIMER_CTC_NOPE, TIMER_CLOCK_1024);
//	timer2_set_compare(97);
	
	TCCR0A = TIMER_WGM_CTC;
	TCCR0B = TIMER_CLOCK_1024;
	TIMSK0 = (0<<OCIE0B)|(1<<OCIE0A)|(0<<TOIE0);
	OCR0A = 80;
	OCR0B = 255;
	
	
	// init SPI slave
	DDRB = (1<<PB3)|(1<<PB4);
	PORTB = (1<<PB3)|(1<<PB4);
	
	while ( 1 )
	{
		sei();
		set_sleep_mode(0);
		sleep_enable();
		sleep_cpu();
	}
	
	return 0;
}
