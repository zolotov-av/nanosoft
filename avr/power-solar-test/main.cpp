
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "iot2313_timer.h"

//ISR(TIMER0_COMPA_vect)
//{
//}

//ISR(TIMER0_COMPB_vect)
//{
//}

//ISR(TIMER0_OVF_vect)
//{
//}

int main()
{
	tim0_init(7, TIM1_CLOCK_1, 0, 2);
	//tim0_set_compareA_interrupt(true);
	//tim0_set_compareB_interrupt(true);
	//tim0_set_capture_interrupt(true);
	//tim0_set_overflow_interrupt(true);
	OCR0A = 100;
	tim0_set_compareA(10);
	
	DDRD |= (1<<PD5);
	
	sei();
	
	set_sleep_mode(0);
	sleep_enable();
	
	while ( 1 )
	{
		sleep_cpu();
	}
	
 	return 0;
}
