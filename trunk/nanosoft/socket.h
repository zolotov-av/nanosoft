#ifndef NANO_SOCKET_H
#define NANO_SOCKET_H

#include <nanosoft/stream.h>

namespace nanosoft
{
	
	class socket: public stream
	{
	private:
		int sock;
		static int inited;
		static int init();
	public:
		socket();
		virtual ~socket();
		bool connect(const char *host, char const *port);
		void close();
		virtual size_t read(void *buffer, size_t size);
		virtual size_t write(const void *buffer, size_t size);
	};
	
}

#endif // NANO_SOCKET_H
