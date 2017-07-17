
#include <nanosoft/options.h>
#include <cstdio>

using namespace nanosoft;

int main(int argc, char **argv)
{
	options opts;
	opts.clear();
	printf("foo: %s\n", opts.read("foo", "123"));
	printf("bar: %s\n", opts.read("bar", "abc"));
	
	opts.write("foo", "hello");
	opts.write("bar", "world");
	printf("foo: %s\n", opts.read("foo", "123"));
	printf("bar: %s\n", opts.read("bar", "abc"));
	
	
	opts.write("foo", "foo");
	opts.remove("bar");
	printf("foo: %s\n", opts.read("foo", "123"));
	printf("bar: %s\n", opts.read("bar", "abc"));
	
	int len = opts.getCount();
	for(int i = 0; i < len; i++)
	{
		printf("%s: %s\n", opts.keyAt(i), opts.valueAt(i));
	}
	
	return 0;
}
