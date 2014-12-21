
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

// 25 at 8MHz = B19200
#define UART_BAUD_K 25

int counter1 = 0;
int counter2 = 0;
unsigned char adc_value = 0;

//ISR(TIMER0_COMP_vect)
//{
//}

//ISR(TIMER0_OVF_vect)
//{
//}

//ISR(TIMER1_COMPA_vect)
//{
//}

ISR(TIMER1_COMPB_vect)
{
}

//ISR(TIMER1_CAPT_vect)
//{
//}

//ISR(TIMER1_OVF_vect)
//{
//}

ISR(ADC_vect)
{
	//adc_value = ADCH;
	UDR = ADCH;
}

int main()
{
	tim1_init(12, TIM0_CLOCK_1, 0, 0);
	tim1_set_compareB_interrupt(true);
	tim1_set_compareB(1000);
	tim1_set_capture(1000);
	
	//DDRA = 0x00;
	DDRA = 0x00;
	PORTA = 0x00;
	
	adc_set_trigger(5);
	ADMUX = (1 << REFS1) | (1 << REFS0) | (1 << ADLAR);
	ADCSRA = (1 << ADEN) | (0 << ADSC) | (1 << ADATE) | (1 << ADIE) | (1 << ADPS2) | (0 << ADPS1) | (1 << ADPS0);
	
	
	// Настройка UART
	UCSRA = 0;
	UCSRB = (1<<RXEN)|(1<<TXEN)|(0<<RXCIE)|(0<<TXCIE)|(0<<UDRIE);
	UCSRC = (1<<URSEL)|(1<<UCSZ0)|(1<<UCSZ1);
	
	// Настройка частоты UART
	UBRRH = (UART_BAUD_K / 256) & ~(1 <<URSEL);
	UBRRL = UART_BAUD_K % 256;
	
	sei();
	
	//set_sleep_mode(0);
	//sleep_enable();
	
	while ( 1 )
	{
		//sleep_cpu();
	}
	
 	return -1;
}
