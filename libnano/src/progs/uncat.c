
/***************************************************************************
* uncat
*
* Простая утилитка, открывает unix-сокет читает из него все что читается
* и выводит в stdout и более ничем непримечательна.
*
****************************************************************************/

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

void std_error()
{
	fprintf(stderr, "Error: %s\n", strerror(errno));
}

int main(int argc, char **argv)
{
	if ( argc < 2 )
	{
		printf("uncat /path/to/socket\n");
		return 0;
	}
	
	// откроем сокет
	int fd = socket(AF_LOCAL, SOCK_STREAM, 0);
	if ( fd == -1 )
	{
		std_error();
		return 1;
	}
	
	int status = 0;
	
	struct sockaddr_un addr;
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, argv[1], sizeof(addr.sun_path)-1);
	addr.sun_path[sizeof(addr.sun_path)-1] = 0;
	
	// подключаемся к unix-сокету
	if ( connect(fd, (struct sockaddr *) &addr, sizeof(addr)) == -1 )
	{
		std_error();
	}
	else
	{
		// писать мы ничего не будем
		if ( shutdown(fd, SHUT_WR) == -1 ) std_error();
		
		int r;
		char buf[4096];
		do
		{
			r = read(fd, buf, sizeof(buf));
			if ( r > 0 ) write(1, buf, r);
		} while ( r > 0 );
		if ( r == -1 )
		{
			std_error();
			status = 2;
		}
		
		if ( shutdown(fd, SHUT_RD) == -1 ) std_error();
	}
	
	if ( close(fd) == -1 ) std_error();
	
	return status;
}
