#ifndef NANO_STREAM_H
#define NANO_STREAM_H

#include <sys/types.h>

namespace nanosoft
{
	
	class stream
	{
	private:
		stream(const stream &) { }
		stream& operator = (const stream &) { }
	public:
		stream() { }
		virtual ~stream() { }
		virtual int read(void *buffer, size_t size) = 0;
		virtual int write(void *buffer, size_t size) = 0;
	};
	
}

#endif // NANO_STREAM_H
