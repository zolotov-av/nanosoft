
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

//#include "pictl.h"
//#include "iom16a_timer.h"
//#include "iom16a_usart.h"
//#include "iom16a_spi.h"
//#include "iom16a_adc.h"

int counter1 = 0;
int counter2 = 0;
unsigned char adc_value = 0;

//ISR(TIMER0_COMP_vect)
//{
//}

//ISR(TIMER0_OVF_vect)
//{
//	counter1 ++;
//	if ( counter1 % 4 == 0 )
//	{
//		counter2 ++;
//		PORTA = (counter2 & 0xFF);
//	}
//}

//ISR(TIMER1_COMPA_vect)
//{
//}

//ISR(TIMER1_COMPB_vect)
//{
//}

//ISR(TIMER1_CAPT_vect)
//{
//}

ISR(TIMER1_OVF_vect)
{
	counter1 ++;
	if ( counter1 % 4000 == 0 )
	{
		counter2 ++;
		PORTA = (counter2 & 0xFF);
	}
}

//ISR(ADC_vect)
//{
//	//adc_value = ADCH;
//	UDR = ADCH;
//}

int main()
{
	//tim0_fast_pwm(TIM0_CLOCK_1024, TIM0_FAST_PWM_NORMAL);
	//tim0_set_overflow_interrupt(true);
 	//tim0_set_compare(128);
	
	//tim1_init(14, TIM1_CLOCK_1, 2, 0);
	//tim1_set_compareA_interrupt(true);
	//tim1_set_compareB_interrupt(true);
	//tim1_set_capture_interrupt(true);
	//tim1_set_overflow_interrupt(true);
	//ICR1 = 100;
	//tim1_set_compareA( 10 );
	//tim1_set_capture(1000);
	
	//DDRA = 0x00;
	//DDRA = 0xFF;
	//PORTA = 0xF0;
	
	//DDRB |= (1<<PB3);
	//DDRD |= (1<<PD5);
	
	//adc_set_trigger(5);
	//ADMUX = (1 << REFS1) | (1 << REFS0) | (1 << ADLAR);
	//ADCSRA = (1 << ADEN) | (0 << ADSC) | (1 << ADATE) | (1 << ADIE) | (1 << ADPS2) | (0 << ADPS1) | (1 << ADPS0);
	
	
	// Настройка UART
	//UCSRA = 0;
	//UCSRB = (1<<RXEN)|(1<<TXEN)|(0<<RXCIE)|(0<<TXCIE)|(0<<UDRIE);
	//UCSRC = (1<<URSEL)|(1<<UCSZ0)|(1<<UCSZ1);
	
	// Настройка частоты UART
	//UBRRH = (UART_BAUD_K / 256) & ~(1 <<URSEL);
	//UBRRL = UART_BAUD_K % 256;
	
	sei();
	
	set_sleep_mode(0);
	sleep_enable();
	
	while ( 1 )
	{
		sleep_cpu();
	}
	
 	return -1;
}
