
#include <nanosoft/socket.h>
#include <unistd.h>
#include <iostream>
#include <exception>
#include <string>

using namespace std;
using namespace nanosoft;

int main(int argc, char **argv)
{
	if ( argc < 2 )
	{
		cout << "minisend host port" << endl;
		return 0;
	}
	
	nanosoft::socket sock;
	
	if ( ! sock.connect(argv[1], argv[2]) )
	{
		cerr << "connect fault" << endl;
		return 1;
	}
	
	while ( 1 )
	{
		char buf[4096];
		int r = read(0, buf, sizeof(buf));
		if ( r > 0 )
		{
			int r2 = sock.write(buf, r);
			if ( r2 < 0 )
			{
				cerr << "send fault" << endl;
				return 1;
			}
		}
		if ( r == 0 ) return 0;
		if ( r < 0 )
		{
			cerr << "read fault" << endl;
			return 1;
		}
	}
	
	return 0;
}
