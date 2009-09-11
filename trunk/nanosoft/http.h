#ifndef NANO_HTTP_H
#define NANO_HTTP_H

#include <nanosoft/socket.h>
#include <nanosoft/options.h>
#include <cstdlib>

namespace nanosoft
{
	
	class http: public socket
	{
	private:
		char *buf;
		int bufsize;
		int length;
		int offset;
	public:
		options headers;
		
		http(int bufsize = 4096);
		virtual ~http();
		bool open(const char *url, const char *method = "GET");
		int getContentLength();
		virtual size_t read(void *buffer, size_t size);
		virtual size_t write(const void *buffer, size_t size);
	};
	
	inline int http::getContentLength()
	{
		return atoi(headers["content-length"]);
	}
}

#endif // NANO_HTTP_H
