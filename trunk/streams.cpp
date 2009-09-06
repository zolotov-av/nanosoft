
#include <nanosoft/fstream.h>
#include <stdio.h>

using namespace nanosoft;

int main(int argc, char **argv)
{
	fstream f;
	f.open("streams.cpp", fstream::ro);
	char buf[4096];
	int s;
	do
	{
		s = f.read(buf, sizeof(buf)-1);
		if ( s > 0 )
		{
			buf[s] = 0;
			printf("%s", buf);
		}
	} while ( s > 0 );
	
	f.close();
	return 0;
}
