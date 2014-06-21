
#include "foo.h"

int Foo::func1(int a, int b)
{
	int c = a + b;
	int d = a - b;
	return c * d;
}

int Foo::func2(int a, int b, int c)
{
	return a * b + c;
}
