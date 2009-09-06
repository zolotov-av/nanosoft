#ifndef NANO_SOCKET_H
#define NANO_SOCKET_H

#include <nanosoft/stream.h>

namespace nanosoft
{
	
	class socket: public stream
	{
	private:
		int sock;
	public:
		socket();
		virtual ~socket();
		bool connect(const char *host, char const *port);
		void close();
		virtual int read(void *buffer, size_t size);
		virtual int write(const void *buffer, size_t size);
	};
	
}

#endif // NANO_SOCKET_H
