
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "pictl.h"
#include "iom16a_timer.h"
#include "iom16a_usart.h"
#include "iom16a_spi.h"

#define TIM2_OCI_ENABLE 1
#define TIM2_TOI_ENABLE 1

int counter = 0;

ISR(TIMER0_COMP_vect)
{
}

ISR(TIMER0_OVF_vect)
{
	counter++;
	PORTA = counter;
	tim0_set_compare(counter);
}

ISR(TIMER1_COMPA_vect)
{
}

ISR(TIMER1_COMPB_vect)
{
}

ISR(TIMER1_CAPT_vect)
{
	counter = tim1_get_capture();
}

ISR(TIMER1_OVF_vect)
{
}

int main()
{
	//tim0_fc_pwm(TIM0_CLOCK_1024);
	tim0_set_overflow_interrupt(true);
	tim0_set_compare_interrupt(true);
	//tim0_set_compare(10);
	
	//timer2_init(TIMER_WGM_CTC, TIMER_CTC_NOPE, TIMER_CLOCK_1024);
	//timer2_set_compare(97);
	
	DDRA = 0xFF;
	PORTA = 0xFF;
	
	while ( 1 )
	{
		sei();
		set_sleep_mode(0);
		sleep_enable();
		sleep_cpu();
	}
	
 	return 0;
}
