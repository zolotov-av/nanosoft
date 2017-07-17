
#include <nanosoft/config.h>
#include <nanosoft/socket.h>
#include <unistd.h>

#ifdef UNIX

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#else

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define SHUT_RDWR SD_BOTH

#endif

#include <fcntl.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>

namespace nanosoft
{
	int socket::inited = socket::init();
	
	int socket::init()
	{
#ifdef WINDOWS
		WORD wVersionRequested = wVersionRequested = MAKEWORD(2, 2);
		WSADATA wsaData;
		int err = WSAStartup(wVersionRequested, &wsaData);
		if ( err != 0 ) fprintf(stderr, "WSAStartup failed with error: %d\n", err);
#endif
		return 1;
	}
	
	socket::socket(): sock(0)
	{
	}
	
	socket::~socket()
	{
		close();
	}
	
	bool socket::connect(const char *host, const char *port)
	{
		// закрыть сокет если открыт
		close();
		
#ifdef USE_GETADDRINFO
		
		struct addrinfo hints, *addr;
		
		// first, load up address structs with getaddrinfo():
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		
		int r = getaddrinfo(host, port, &hints, &addr);
		if ( r == 0 )
		{
			int s = ::socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
			
			if ( s > 0 )
			{
				if ( ::connect(s, addr->ai_addr, addr->ai_addrlen) == 0 )
				{
					sock = s;
					return 1;
				}
				else
				{
					fprintf(stderr, "connect() fault\n");
				}
				
				::close(s);
			}
			else
			{
				fprintf(stderr, "socket() fault\n");
			}
			
			freeaddrinfo(addr);
		}
		else
		{
#ifdef HAVE_GAI_STRERROR
			fprintf(stderr, "getaddrinfo() fault: %s\n", gai_strerror(r));
#else
			fprintf(stderr, "getaddrinfo() fault\n");
#endif
		}
		
#else
		
		struct hostent *he = gethostbyname(host);
		if ( he == 0 ) return 0;
		
		struct sockaddr_in target;
		target.sin_family = AF_INET;
		target.sin_port = htons( (u_short)atoi(port) );
		
		if( he->h_length != sizeof( struct in_addr ) )
		{
			fprintf(stderr, "he->h_length != sizeof( struct in_addr )\n");
			return 0;
		}
		else
		{
			memcpy(&target.sin_addr, he->h_addr, sizeof( struct in_addr ));
		}
		
		memset(target.sin_zero, '\0', 8);
		
		int s = ::socket(PF_INET, SOCK_STREAM, 0);
		
		if ( s > 0 )
		{
			if ( ::connect(s, (struct sockaddr *)&target, sizeof( struct sockaddr )) == 0 )
			{
				sock = s;
				return 1;
			}
			else
			{
				fprintf(stderr, "connect() fault\n");
			}
			
			::close(s);
		}
		else
		{
			fprintf(stderr, "socket() fault\n");
		}
		
#endif
		return 0;
	}
	
	void socket::close()
	{
		if ( sock )
		{
			::shutdown(sock, SHUT_RDWR);
			::close(sock);
		}
	}
	
	size_t socket::read(void *buffer, size_t size)
	{
		return ::recv(sock, static_cast<char*>(buffer), size, 0);
	}
	
	size_t socket::write(const void *buffer, size_t size)
	{
		return ::send(sock, static_cast<const char*>(buffer), size, 0);
	}
	
}
