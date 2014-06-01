
//#define F_CPU 1000000UL  // 1 MHz

#include <avr/io.h>
#include <avr/interrupt.h>
#include "foo.h"
// #include <util/delay.h>

int banana(int a, int b)
{
	int c = a + b;
	int d = a - b;
	return c * d;
}

int main()
{
	return foo(1, 4);
}
