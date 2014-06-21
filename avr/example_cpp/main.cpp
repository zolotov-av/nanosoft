
//#define F_CPU 1000000UL  // 1 MHz

#include <avr/io.h>
#include <avr/interrupt.h>
#include "foo.h"
#include "ringbuf.h"


int main()
{
	RingBuf buf;
	while ( 1 )
	{
		if ( ! buf.isEmpty() )
		{
			buf.fetch();
		}
	}
	return 0;
}
