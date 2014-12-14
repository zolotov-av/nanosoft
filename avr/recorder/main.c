
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

// 51 at 8MHz = 9600
#define UART_BAUD_K 51

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
	
	ADMUX = (1 << ADLAR);
	ADCSRA = (0 << ADEN) | (0 << ADSC) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (0 << ADPS0);
	SFIOR = 0;	
	
	// Настройка прочих портов
	DDRA = 0x00; // АЦП
	DDRC = 0xFF; // тест
	PORTC = 0xFF; 
	
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
