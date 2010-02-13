
#include <nanosoft/mparser.h>
#include <stdio.h>

using namespace nanosoft;

int main()
{
	// f(x) = sin(x) * cos(x)
	MathFunction<double> f = sin<double>() * cos<double>();
	
	// g(x) = f'(x)
	MathFunction<double> g = f.derive();
	
	printf("f(x) = %s\n", f.toString().c_str());
	printf("g(x) = %s\n", g.toString().c_str());
	
	printf("f(1): %0.3f should %0.3f\n", f(1), sin(1) * cos(1));
	printf("g(1): %0.3f should %0.3f\n", g(1), cos(1)*cos(1) - sin(1)*sin(1));
	
	return 0;
}
