
#include "mod_foo.h"

extern "C" int sum(int a, int b)
{
	return a + b;
}

extern "C" int mul(int a, int b)
{
	return a * b;
}
