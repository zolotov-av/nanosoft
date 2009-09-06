#ifndef NANO_HTTP_H
#define NANO_HTTP_H

#include <nanosoft/socket.h>

namespace nanosoft
{
	
	class http: public socket
	{
	private:
		char *buf;
		int bufsize;
		int length;
		int offset;
		int contentLength;
	public:
		http(int bufsize = 4096);
		virtual ~http();
		bool open(const char *url, const char *method = "GET");
		int getContentLength();
		virtual int read(void *buffer, size_t size);
		virtual int write(const void *buffer, size_t size);
	};
	
	inline int http::getContentLength()
	{
		return contentLength;
	}
}

#endif // NANO_HTTP_H
