
#include <nanosoft/math.h>
#include <nanosoft/mathparser.h>
#include <stdio.h>
#include <exception>

using namespace std;
using namespace nanosoft;

int count = 0, passed = 0;

// Объявляем переменные
MathVar x("x"), y("y"), z("z");

// Объявляем параметры
MathVar R("R"), a("a"), b("b");

// парсер
MathParser p;

/**
* Проверка построения функции в коде
*/
void test_runtime(int line, const MathFunction &f, const std::string &should)
{
	count++;
	bool ok = false;
	try
	{
		string fs = f.toString();
		ok = fs == should;
		printf("%5d: RUNTIME: %s SHOULD %s [ %s ]\n", line, fs.c_str(), should.c_str(), (ok ? " OK " : "FAIL"));
	}
	catch(exception e)
	{
		fprintf(stderr, "exception: %s\n", e.what());
	}
	if ( ok ) passed ++;
}

void test(const char *name, double result, double should)
{
	const char *status = (result == should) ? "OK" : "FAULT";
	printf("%s: %0.3f should %0.3f [ %s ]\n", name, result, should, status);
}

void test_runtime()
{
	printf("\nbasic:\n");
	test_runtime(__LINE__, x, "x");
	test_runtime(__LINE__, -x, "-(x)");
	
	printf("\nsums:\n");
	test_runtime(__LINE__, x + y, "x + y");
	test_runtime(__LINE__, x - y, "x - y");
	test_runtime(__LINE__, -x - y, "-(x) - y");
	test_runtime(__LINE__, x + y + z, "x + y + z");
	test_runtime(__LINE__, x + y - z, "x + y - z");
	test_runtime(__LINE__, x - y + z, "x - y + z");
	test_runtime(__LINE__, x - y - z, "x - y - z");
	test_runtime(__LINE__, - x - y - z, "-(x) - y - z");
	
	printf("\nmults:\n");
	test_runtime(__LINE__, x * y, "(x) * (y)");
	test_runtime(__LINE__, x + y * z, "x + (y) * (z)");
	test_runtime(__LINE__, x * y + z, "(x) * (y) + z");
	test_runtime(__LINE__, x * y * z, "((x) * (y)) * (z)");
	test_runtime(__LINE__, x * (y * z), "(x) * ((y) * (z))");
	test_runtime(__LINE__, (x + y) * z, "(x + y) * (z)");
	test_runtime(__LINE__, x * (y + z), "(x) * (y + z)");
	test_runtime(__LINE__, (x - y) * (x + y), "(x - y) * (x + y)");
	
	printf("\ndivs:\n");
	test_runtime(__LINE__, x / y, "(x) / (y)");
	test_runtime(__LINE__, x + y / z, "x + (y) / (z)");
	test_runtime(__LINE__, x / y + z, "(x) / (y) + z");
	test_runtime(__LINE__, (x * y) / z, "((x) * (y)) / (z)");
	test_runtime(__LINE__, x * (y / z), "(x) * ((y) / (z))");
	test_runtime(__LINE__, (x / y) / z, "((x) / (y)) / (z)");
	test_runtime(__LINE__, x / (y / z), "(x) / ((y) / (z))");
	
	printf("\nsin/cos:\n");
	test_runtime(__LINE__, sin(x), "sin(x)");
	test_runtime(__LINE__, cos(x), "cos(x)");
	
	printf("\nexp/log:\n");
	test_runtime(__LINE__, exp(x), "exp(x)");
	test_runtime(__LINE__, pow(a, x), "(a)^(x)");
	test_runtime(__LINE__, ln(x), "ln(x)");
	test_runtime(__LINE__, log(x, a), "(ln(x)) / (ln(a))");
}

/**
* Проверка парсера
*/
void test_parser(int line, const char *expr, const char *should)
{
	count++;
	bool ok = false;
	try
	{
		MathFunction f = p.parse(expr);
		string fs = f.toString();
		ok = fs == should;
		printf("%5d: PARSER: %s [ %s ]\n", line, expr, (ok ? " OK " : "FAIL"));
		if ( ! ok )
		{
			printf("       SHOULD %s\n", should);
			printf("       BUT %s\n", fs.c_str());
		}
	}
	catch(MathParserError e)
	{
		printf("       EXCEPTION: %s\n", e.what());
	}
	if ( ok ) passed ++;
}

void test_parser()
{
	printf("\nparser sums:\n");
	test_parser(__LINE__, "x", "x");
	test_parser(__LINE__, "-x", "-(x)");
	test_parser(__LINE__, "x + y", "x + y");
	test_parser(__LINE__, "x + y + z", "x + y + z");
	test_parser(__LINE__, "x - y", "x - y");
	test_parser(__LINE__, "x - y + z", "x - y + z");
	test_parser(__LINE__, "x + y - z", "x + y - z");
	test_parser(__LINE__, "x - y - z", "x - y - z");
	test_parser(__LINE__, "- x - y - z", "-(x) - y - z");
	
	printf("\nparser mults:\n");
	test_parser(__LINE__, "x * y", "(x) * (y)");
	test_parser(__LINE__, "x * y * z", "((x) * (y)) * (z)");
	test_parser(__LINE__, "x + y * z", "x + (y) * (z)");
	test_parser(__LINE__, "x * y + z", "(x) * (y) + z");
	test_parser(__LINE__, "(x + y) * z", "(x + y) * (z)");
	test_parser(__LINE__, "x * (y + z)", "(x) * (y + z)");
	test_parser(__LINE__, "(x - y) * (x + y)", "(x - y) * (x + y)");
	test_parser(__LINE__, "x * y + y * z", "(x) * (y) + (y) * (z)");
	
	printf("\nparser divs:\n");
	test_parser(__LINE__, "x / y", "(x) / (y)");
	test_parser(__LINE__, "x / y / z", "((x) / (y)) / (z)");
	test_parser(__LINE__, "x + y / z", "x + (y) / (z)");
	test_parser(__LINE__, "x / y + z", "(x) / (y) + z");
	test_parser(__LINE__, "(x + y) / z", "(x + y) / (z)");
	test_parser(__LINE__, "x / (y + z)", "(x) / (y + z)");
	test_parser(__LINE__, "(x - y) / (x + y)", "(x - y) / (x + y)");
	test_parser(__LINE__, "x * y / z", "((x) * (y)) / (z)");
	test_parser(__LINE__, "x / y * z", "((x) / (y)) * (z)");
	
	printf("\nsin/cos:\n");
	test_parser(__LINE__, "sin(x)", "sin(x)");
	test_parser(__LINE__, "cos(x)", "cos(x)");
	test_parser(__LINE__, "sin(x) + cos(y)", "sin(x) + cos(y)");
	test_parser(__LINE__, "sin(x + y)", "sin(x + y)");
	test_parser(__LINE__, "cos(x * y)", "cos((x) * (y))");
	
	printf("\nexp/log:\n");
	test_parser(__LINE__, "exp(x)", "exp(x)");
	test_parser(__LINE__, "ln(x)", "ln(x)");
	test_parser(__LINE__, "log(x, a)", "(ln(x)) / (ln(a))");
	test_parser(__LINE__, "exp(x + y)", "exp(x + y)");
	test_parser(__LINE__, "ln(x * y)", "ln((x) * (y))");
	test_parser(__LINE__, "pow(x, y)", "(x)^(y)");
	test_parser(__LINE__, "pow(x + y, a - b)", "(x + y)^(a - b)");
}

int main()
{
	p.setVar("x", x);
	p.setVar("y", y);
	p.setVar("z", z);
	p.setVar("R", R);
	p.setVar("a", a);
	p.setVar("b", b);
	
	test_runtime();
	test_parser();
	
	printf("\nPASSED: %d/%d\n", count, passed);
	if ( count == passed ) printf("OK\n");
	return (count == passed) ? 0 : 1;
}
