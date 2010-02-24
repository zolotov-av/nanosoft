
#include <nanosoft/math.h>
#include <nanosoft/mathparser.h>
#include <stdio.h>

using namespace nanosoft;

void test(const char *name, double result, double should)
{
	const char *status = (result == should) ? "OK" : "FAULT";
	printf("%s: %0.3f should %0.3f [ %s ]\n", name, result, should, status);
}

int main()
{
	// Объявляем переменные
	MathVar x("x"), y("y");
	
	MathParser p;
	p.vars["x"] = x;
	p.vars["y"] = y;
	p.funcx["cos"] = cos;
	p.funcx["sin"] = sin;
	
	// f(x, y) = sin(x) * cos(y)
	MathFunction f = p.parse("sin(x) * cos(y)");
	printf("f(x,y) = %s\n", f.toString().c_str());
	
	// g(x,y) = f'x(x,y)
	MathFunction g = f.derive(x).optimize();
	printf("g(x,y) = %s\n", g.toString().c_str());
	
	// t(x,y) = f'y(x,y)
	MathFunction t = (f.derive(y)).optimize();
	printf("t(x,y) = %s\n\n", t.toString().c_str());
	
	x.setValue(1);
	y.setValue(2);
	test("f(1,2)", f.eval(), sin(x.getValue()) * cos(y.getValue()));
	test("g(1,2)", g.eval(), cos(x.getValue()) * cos(y.getValue()));
	test("t(1,2)", t.eval(), - sin(x.getValue()) * sin(y.getValue()));
	printf("\n");
	
	x.setValue(2);
	y.setValue(1);
	test("f(2,1)", f.eval(), sin(x.getValue()) * cos(y.getValue()));
	test("g(2,1)", g.eval(), cos(x.getValue()) * cos(y.getValue()));
	test("t(2,1)", t.eval(), - sin(x.getValue()) * sin(y.getValue()));
	printf("\n");
	/* */
	return 0;
}
