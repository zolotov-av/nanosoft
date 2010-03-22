
#include <nanosoft/string.h>
#include <stdio.h>

using namespace nanosoft;

int main(int argc, char **argv)
{
	tstrbuf<7> buf = "  Hello world  ";
	printf("buf: '%s'\n", buf.c_str());
	printf("buf == 'Hello': %s\n", buf == "Hello" ? "yes" : "no");
	return 0;
}
