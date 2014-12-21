
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "pictl.h"
#include "iom16a_timer.h"
#include "iom16a_usart.h"
#include "iom16a_spi.h"
#include "iom16a_adc.h"

#define TIM2_OCI_ENABLE 1
#define TIM2_TOI_ENABLE 1

int counter = 0;

ISR(TIMER0_COMP_vect)
{
}

ISR(TIMER0_OVF_vect)
{
	//ADCSRA |= (1 << ADSC);
}

ISR(TIMER1_COMPA_vect)
{
}

ISR(TIMER1_COMPB_vect)
{
}

ISR(TIMER1_CAPT_vect)
{
}

ISR(TIMER1_OVF_vect)
{
}

ISR(ADC_vect)
{
	counter++;
	PORTA = (counter >> 8);
}

int main()
{
	
	//tim0_normal(TIM1_CLOCK_1024);
	//tim0_set_overflow_interrupt(true);
	
	DDRA = 0xFF;
	PORTA = 0xFF;
	
	ADMUX = 0;
	ADCSRA = 0x8E;
	adc_set_trigger(0);
	sei();
	
	set_sleep_mode(0);
	sleep_enable();
		
	while ( 1 )
	{
		sleep_cpu();
	}
	
 	return -1;
}
