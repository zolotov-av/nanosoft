
#include "ringbuf.h"

RingBuf::RingBuf(): len(0), offset(0)
{
}

bool RingBuf::put(char ch)
{
	if ( len < sizeof(buf) )
	{
		buf[ (offset + len++) % sizeof(buf) ] = ch;
		return true;
	}
	return false;
}

char RingBuf::fetch()
{
	if ( len > 0 )
	{
		char ch = buf[offset];
		offset = (offset + 1) % sizeof(buf);
		return ch;
	}
	return 0;
}
