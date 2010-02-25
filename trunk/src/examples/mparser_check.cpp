
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

/**
* Проверка оптимизатора
*/
void test_optimizer(int line, const MathFunction &f, const MathFunction &opt)
{
	count++;
	bool ok = false;
	try
	{
		MathFunction g = optimize(f);
		
		string orig = f.toString();
		string gets = g.toString();
		string should = opt.toString();
		
		ok = (gets == should);
		
		printf("%5d: OPTIMIZER: %s [ %s ]\n", line, orig.c_str(), (ok ? " OK " : "FAIL"));
		if ( ! ok )
		{
			printf("       SHOULD %s\n", should.c_str());
			printf("       BUT %s\n", gets.c_str());
		}
	}
	catch(MathParserError e)
	{
		printf("       EXCEPTION: %s\n", e.what());
	}
	if ( ok ) passed ++;
}

void test_optimizer()
{
	printf("\noptimizer do not break function:\n");
	test_optimizer(__LINE__, x, x);
	test_optimizer(__LINE__, -x, -x);
	test_optimizer(__LINE__, x + y, x + y);
	test_optimizer(__LINE__, x + y + z, x + y + z);
	test_optimizer(__LINE__, x - y, x - y);
	test_optimizer(__LINE__, x - y - z, x - y - z);
	test_optimizer(__LINE__, x * y, x * y);
	test_optimizer(__LINE__, x * y * z, x * y * z);
	test_optimizer(__LINE__, x + y * z, x + y * z);
	test_optimizer(__LINE__, x * y + z, x * y + z);
	test_optimizer(__LINE__, x / y, x / y);
	test_optimizer(__LINE__, x / y / z, x / y / z);
	test_optimizer(__LINE__, x + y / z, x + y / z);
	test_optimizer(__LINE__, x / y + z, x / y + z);
	test_optimizer(__LINE__, x * y / z, x * y / z);
	test_optimizer(__LINE__, x / y * z, x / y * z);
	test_optimizer(__LINE__, sin(x), sin(x));
	test_optimizer(__LINE__, cos(x), cos(x));
	test_optimizer(__LINE__, exp(x), exp(x));
	test_optimizer(__LINE__, pow(x, y), pow(x, y));
	test_optimizer(__LINE__, ln(x), ln(x));
	test_optimizer(__LINE__, log(x, y), log(x, y));
	
	printf("\nsum optimizer:\n");
	test_optimizer(__LINE__, x + 0, x);
	test_optimizer(__LINE__, 0 + x, x);
	test_optimizer(__LINE__, x + 1, x + 1);
	test_optimizer(__LINE__, 1 + x, x + 1);
	test_optimizer(__LINE__, x + 1 + 2, x + 3);
	test_optimizer(__LINE__, 1 + x + 2, x + 3);
	test_optimizer(__LINE__, 1 + 2 + x, x + 3);
	test_optimizer(__LINE__, 1 + x + y, x + y + 1);
	test_optimizer(__LINE__, x + 1 + y, x + y + 1);
	test_optimizer(__LINE__, x + y + 1, x + y + 1);
	test_optimizer(__LINE__, 1 + x + 2 + y + 3, x + y + 6);
	
	printf("\nmult optimizer:\n");
	test_optimizer(__LINE__, x * 0.0, 0.0);
	test_optimizer(__LINE__, 0.0 * x, 0.0);
	test_optimizer(__LINE__, x * 1, x);
	test_optimizer(__LINE__, 1 * x, x);
	test_optimizer(__LINE__, x * 2, x * 2);
	test_optimizer(__LINE__, 2 * x, x * 2);
	test_optimizer(__LINE__, x * 1 * 2, x * 2);
	test_optimizer(__LINE__, 1 * x * 2, x * 2);
	test_optimizer(__LINE__, 1 * 2 * x, x * 2);
	test_optimizer(__LINE__, 1 * x * y, x * y);
	test_optimizer(__LINE__, x * 1 * y, x * y);
	test_optimizer(__LINE__, x * y * 1, x * y);
	test_optimizer(__LINE__, 1 * x * 2 * y * 3, x * y * 6);
	
	printf("\nneg optimizer:\n");
	test_optimizer(__LINE__, -x, -x);
	test_optimizer(__LINE__, - x + y, y - x);
	test_optimizer(__LINE__, x - y, x - y);
	test_optimizer(__LINE__, x - y - z, x - y - z);
	test_optimizer(__LINE__, - x + y + z, y - x + z);
	
	printf("\nsin/cos optimizer:\n");
	test_optimizer(__LINE__, cos(x), cos(x));
	test_optimizer(__LINE__, cos(-x), cos(x));
	test_optimizer(__LINE__, sin(x), sin(x));
	test_optimizer(__LINE__, sin(-x), -sin(x));
	test_optimizer(__LINE__, sin(-x) * sin(-y), sin(x) * sin(y));
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
	test_optimizer();
	
	printf("\nPASSED: %d/%d\n", count, passed);
	if ( count == passed ) printf("OK\n");
	return (count == passed) ? 0 : 1;
}
