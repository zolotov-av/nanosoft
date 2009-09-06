
#include <nanosoft/http.h>

extern "C"
{
#include <nanourl.h>
#include <nanostr.h>
}

#include <stdlib.h>
#include <string.h>

#include <stdio.h>


namespace nanosoft
{
	
	http::http(int abufsize):
		buf(new char[abufsize]),
		bufsize(abufsize),
		length(0),
		offset(0)
	{
	}
	
	http::~http()
	{
		delete[] buf;
	}
	
	bool http::open(const char *url, const char *method)
	{
		char request[4096];
		url_p u = url_parse(url);
		if ( u == 0 ) return 0;
		
		if ( connect(u->host, u->port ? u->port : "80") )
		{
			sprintf(request, "%s %s HTTP/1.0\r\nhost: %s%s%s\r\n\r\n",
				method,
				u->path,
				u->host,
				u->query ? "?" : "",
				u->query ? u->query : ""
			);
			write(request, strlen(request));
			
			// read headers
			length = socket::read(buf, bufsize-1);
			offset = 0;
			
			if ( length <= 0 )
			{
				fprintf(stderr, "nanosoft::http::open(%s): read headers fault\n", url);
				url_free(u);
				close();
				return 0;
			}
			
			char *body = strstr(buf, "\r\n\r\n");
			buf[length] = 0;
			if ( body == 0 )
			{
				fprintf(stderr, "nanosoft::http::open(%s): headers too large\n", url);
				url_free(u);
				close();
				return 0;
			}
			
			offset = body - static_cast<char*>(buf) + 4;
			body += 2;
			
			char *p = buf, *end, *value;
			contentLength = -1;
			while ( (p < body) && (end = strchr(p, '\n')) )
			{
				*end = 0;
				char *value = strchr(p, ':');
				if ( value == 0 ) value = end;
				else
				{
					*value = 0;
					value++;
				}
				strtolower(trim(p));
				if ( strcmp(p, "content-length") == 0 )
				{
					contentLength = atoi(trim(value));
				}
				p = end + 1;
			}
			
			return 1;
		}
		
		url_free(u);
		return 0;
	}
	
	static int min(int a, int b)
	{
		return a < b ? a : b;
	}
	
	ssize_t http::read(void *buffer, size_t size)
	{
		if ( size == 0 ) return 0;
		
		char *dest = static_cast<char *>(buffer);
		int toread = 0;
		int done = 0;
		
		if ( length > offset )
		{
			toread = min(length - offset, size);
			memcpy(dest, buf + offset, toread);
			dest += toread;
			size -= toread;
			done += toread;
			offset += toread;
			if ( size == 0 ) return done;
		}
		
		if ( size >= bufsize ) return socket::read(dest, size) + done;
		
		length = socket::read(buf, bufsize);
		offset = 0;
		toread = min(size, length);
		memcpy(dest, buf, toread);
		done += toread;
		offset += toread;
		return done;
	}
	
	ssize_t http::write(const void *buffer, size_t size)
	{
		return socket::write(buffer, size);
	}
	
}
