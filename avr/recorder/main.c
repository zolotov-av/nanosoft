
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

// 51 at 8MHz = 9600
#define UART_BAUD_K 51

static char map[] = "0123456789ABCDEF";

ISR(ADC_vect)
{
	char byte = ADCH;
	char lo = byte & 0x0F;
	char hi = (byte >> 4) & 0x0F;
	usart_putc(map[hi]);
	usart_putc(map[lo]);
	usart_putc('\r');
	usart_putc('\n');
	//ADCSRA &= ~(1 << ADEN);
}

char data;

int main()
{
	// Настройка UART
	UCSRA = 0;
	UCSRB = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE)|(1<<TXCIE)|(1<<UDRIE);
	UCSRC = (1<<URSEL)|(1<<UCSZ0)|(1<<UCSZ1);
	
	// Настройка частоты UART
	UBRRH = (UART_BAUD_K / 256) & ~(1 <<URSEL);
	UBRRL = UART_BAUD_K % 256;
	
	TCCR0 = (1 << WGM01) | (0 << WGM00) | (1 << CS02) | (0 << CS01) | (1 << CS00);
	TIMSK = 0;
	OCR0 = 200;
	TCNT0 = 0;
	
	ADMUX = (1 << ADLAR);
	ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADATE) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (0 << ADPS0);
	SFIOR = (0 << ADTS2) | (1 << ADTS1) | (1 << ADTS0);
	
	// Настройка прочих портов
	DDRA = 0x00; // АЦП
	//DDRC = 0xFF; // тест
	//PORTC = 0xFF;
	
	
	while ( 1 )
	{
		sei();
		set_sleep_mode(0);
		sleep_enable();
		sleep_cpu();
		while ( usart_getc(&data) )
		{
			usart_putc(data);
		}
	}
	
	return 0;
}
