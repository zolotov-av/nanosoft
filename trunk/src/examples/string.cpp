
#include <nanosoft/string.h>
#include <cstdio>

using namespace nanosoft;

int main(int argc, char **argv)
{
	cstring empty;
	printf("empty: %s\n", empty.c_str());
	
	cstring hello("Hello world");
	printf("hello: %s\n", hello.c_str());
	
	hello = hello;
	
	hello.copy(hello, 0, 5);
	printf("1 (Hello): %s\n", hello.c_str());
	
	printf("2 (He): %s\n", substr(hello, -1, 3).c_str());
	
	printf("3 (null): %s\n", substr(hello, -10, 2).c_str());
	
	printf("4 (null): %s\n", substr(hello, 10, 2).c_str());
	
	printf("5 (lo): %s\n", substr(hello, 3, 10).c_str());
	
	hello = "test";
	printf("6 (test): %s\n", hello.c_str());
	
	empty = hello;
	printf("7 (test): %s\n", empty.c_str());
	
	empty.trim(" \r\n\tempty\r\t\n ");
	empty.trim(empty);
	printf("8 (empty): %s\n", empty.c_str());
	
	printf("9 (empty): %s\n", empty.c_str());
	
	empty.trim(hello);
	printf("10 (test): %s\n", empty.c_str());
	
	return 0;
}
