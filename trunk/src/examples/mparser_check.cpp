
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
		string fs = f.debugString();
		ok = fs == should;
		printf("%5d: RUNTIME: %s SHOULD %s [ %s ]\n", line, fs.c_str(), should.c_str(), (ok ? " OK " : "FAIL"));
	}
	catch(exception e)
	{
		fprintf(stderr, "exception: %s\n", e.what());
	}
	if ( ok ) passed ++;
}

void test_runtime()
{
	printf("\nbasic:\n");
	test_runtime(__LINE__, x, "x");
	test_runtime(__LINE__, -x, "neg(x)");
	
	printf("\nsums:\n");
	test_runtime(__LINE__, x + y, "(x + y)");
	test_runtime(__LINE__, x - y, "(x - y)");
	test_runtime(__LINE__, -x - y, "(neg(x) - y)");
	test_runtime(__LINE__, x + y + z, "((x + y) + z)");
	test_runtime(__LINE__, x + y - z, "((x + y) - z)");
	test_runtime(__LINE__, x - y + z, "((x - y) + z)");
	test_runtime(__LINE__, x - y - z, "((x - y) - z)");
	test_runtime(__LINE__, - x - y - z, "((neg(x) - y) - z)");
	
	printf("\nmults:\n");
	test_runtime(__LINE__, x * y, "(x * y)");
	test_runtime(__LINE__, x + y * z, "(x + (y * z))");
	test_runtime(__LINE__, x * y + z, "((x * y) + z)");
	test_runtime(__LINE__, x * y * z, "((x * y) * z)");
	test_runtime(__LINE__, x * (y * z), "(x * (y * z))");
	test_runtime(__LINE__, (x + y) * z, "((x + y) * z)");
	test_runtime(__LINE__, x * (y + z), "(x * (y + z))");
	test_runtime(__LINE__, (x - y) * (x + y), "((x - y) * (x + y))");
	
	printf("\ndivs:\n");
	test_runtime(__LINE__, x / y, "(x / y)");
	test_runtime(__LINE__, x + y / z, "(x + (y / z))");
	test_runtime(__LINE__, x / y + z, "((x / y) + z)");
	test_runtime(__LINE__, (x * y) / z, "((x * y) / z)");
	test_runtime(__LINE__, x * (y / z), "(x * (y / z))");
	test_runtime(__LINE__, (x / y) / z, "((x / y) / z)");
	test_runtime(__LINE__, x / (y / z), "(x / (y / z))");
	
	printf("\nsin/cos:\n");
	test_runtime(__LINE__, sin(x), "sin(x)");
	test_runtime(__LINE__, cos(x), "cos(x)");
	
	printf("\nexp/log:\n");
	test_runtime(__LINE__, exp(x), "exp(x)");
	test_runtime(__LINE__, pow(a, x), "(a ^ x)");
	test_runtime(__LINE__, ln(x), "ln(x)");
	test_runtime(__LINE__, log(x, a), "(ln(x) / ln(a))");
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
		string fs = f.debugString();
		ok = fs == should;
		printf("%5d: PARSER: %s [ %s ]\n", line, expr, (ok ? " OK " : "FAIL"));
		if ( ! ok )
		{
			printf("       SHOULD: %s\n", should);
			printf("          BUT: %s\n", fs.c_str());
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
	test_parser(__LINE__, "-x", "neg(x)");
	test_parser(__LINE__, "x + y", "(x + y)");
	test_parser(__LINE__, "x + y + z", "((x + y) + z)");
	test_parser(__LINE__, "x - y", "(x - y)");
	test_parser(__LINE__, "x - y + z", "((x - y) + z)");
	test_parser(__LINE__, "x + y - z", "((x + y) - z)");
	test_parser(__LINE__, "x - y - z", "((x - y) - z)");
	test_parser(__LINE__, "- x - y - z", "((neg(x) - y) - z)");
	
	printf("\nparser mults:\n");
	test_parser(__LINE__, "x * y", "(x * y)");
	test_parser(__LINE__, "x * y * z", "((x * y) * z)");
	test_parser(__LINE__, "x + y * z", "(x + (y * z))");
	test_parser(__LINE__, "x * y + z", "((x * y) + z)");
	test_parser(__LINE__, "(x + y) * z", "((x + y) * z)");
	test_parser(__LINE__, "x * (y + z)", "(x * (y + z))");
	test_parser(__LINE__, "(x - y) * (x + y)", "((x - y) * (x + y))");
	test_parser(__LINE__, "x * y + y * z", "((x * y) + (y * z))");
	
	printf("\nparser divs:\n");
	test_parser(__LINE__, "x / y", "(x / y)");
	test_parser(__LINE__, "x / y / z", "((x / y) / z)");
	test_parser(__LINE__, "x + y / z", "(x + (y / z))");
	test_parser(__LINE__, "x / y + z", "((x / y) + z)");
	test_parser(__LINE__, "(x + y) / z", "((x + y) / z)");
	test_parser(__LINE__, "x / (y + z)", "(x / (y + z))");
	test_parser(__LINE__, "(x - y) / (x + y)", "((x - y) / (x + y))");
	test_parser(__LINE__, "x * y / z", "((x * y) / z)");
	test_parser(__LINE__, "x / y * z", "((x / y) * z)");
	
	printf("\nsin/cos:\n");
	test_parser(__LINE__, "sin(x)", "sin(x)");
	test_parser(__LINE__, "cos(x)", "cos(x)");
	test_parser(__LINE__, "sin(x) + cos(y)", "(sin(x) + cos(y))");
	test_parser(__LINE__, "sin(x + y)", "sin((x + y))");
	test_parser(__LINE__, "cos(x * y)", "cos((x * y))");
	
	printf("\nexp/log:\n");
	test_parser(__LINE__, "exp(x)", "exp(x)");
	test_parser(__LINE__, "ln(x)", "ln(x)");
	test_parser(__LINE__, "log(x, a)", "(ln(x) / ln(a))");
	test_parser(__LINE__, "exp(x + y)", "exp((x + y))");
	test_parser(__LINE__, "ln(x * y)", "ln((x * y))");
	test_parser(__LINE__, "pow(x, y)", "(x ^ y)");
	test_parser(__LINE__, "pow(x + y, a - b)", "((x + y) ^ (a - b))");
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
		
		string orig = f.debugString();
		string gets = g.debugString();
		string should = opt.debugString();
		
		ok = (gets == should);
		
		printf("%5d: OPTIMIZER: %s => %s [ %s ]\n", line, orig.c_str(), should.c_str(), (ok ? " OK " : "FAIL"));
		if ( ! ok )
		{
			printf("          SHOULD: %s\n", should.c_str());
			printf("             BUT: %s\n", gets.c_str());
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
	printf("\noptimize[nop]:\n");
	test_optimizer(__LINE__, x, x);
	
	printf("\noptimizer do not break function:\n");
	test_optimizer(__LINE__, -x, -x);
	test_optimizer(__LINE__, x + y, x + y);
	test_optimizer(__LINE__, x + y + z, x + y + z);
	//test_optimizer(__LINE__, x - y, x - y);
	//test_optimizer(__LINE__, x - y - z, x - y - z);
	test_optimizer(__LINE__, x * y, x * y);
	test_optimizer(__LINE__, x * y * z, x * y * z);
	test_optimizer(__LINE__, x + y * z, x + y * z);
	test_optimizer(__LINE__, x * y + z, x * y + z);
	//test_optimizer(__LINE__, x / y, x / y);
	//test_optimizer(__LINE__, x / y / z, x / y / z);
	//test_optimizer(__LINE__, x + y / z, x + y / z);
	//test_optimizer(__LINE__, x / y + z, x / y + z);
	//test_optimizer(__LINE__, x * y / z, x * y / z);
	//test_optimizer(__LINE__, x / y * z, x / y * z);
	test_optimizer(__LINE__, sin(x), sin(x));
	test_optimizer(__LINE__, cos(x), cos(x));
	test_optimizer(__LINE__, exp(x), exp(x));
	test_optimizer(__LINE__, pow(x, y), pow(x, y));
	test_optimizer(__LINE__, ln(x), ln(x));
	//test_optimizer(__LINE__, log(x, y), log(x, y));
	
	printf("\nsin/cos optimizer:\n");
	test_optimizer(__LINE__, cos(MathFunction(0.0)), 1.0);
	test_optimizer(__LINE__, sin(MathFunction(0.0)), 0.0);
	test_optimizer(__LINE__, exp(MathFunction(0.0)), 1.0);
	test_optimizer(__LINE__, ln(exp(MathFunction(1.0))), 1.0);
	test_optimizer(__LINE__, cos(x), cos(x));
	test_optimizer(__LINE__, cos(-x), cos(x));
	test_optimizer(__LINE__, sin(x), sin(x));
	test_optimizer(__LINE__, sin(-x), -sin(x));
	test_optimizer(__LINE__, sin(-x) * sin(-y), sin(x) * sin(y));
	
	// new tests
	printf("\noptimize[sum]:\n");
	test_optimizer(__LINE__, MathFunction(1.0) + 2.0, 3.0);
	test_optimizer(__LINE__, 0.0 + x, x);
	test_optimizer(__LINE__, 1.0 + x, 1.0 + x);
	test_optimizer(__LINE__, 1.0 + (2.0 + x), 3.0 + x);
	test_optimizer(__LINE__, 1.0 + (-1.0 + x), x);
	printf("\n");
	test_optimizer(__LINE__, x + 0.0, x);
	test_optimizer(__LINE__, x + 1.0, 1.0 + x);
	test_optimizer(__LINE__, (x + 1.0) + 2.0, 3.0 + x);
	test_optimizer(__LINE__, (x + (-1.0)) + 1.0, x);
	printf("\n");
	test_optimizer(__LINE__, (1.0 + x) + (-1.0 + y), x + y);
	test_optimizer(__LINE__, (1.0 + x) + (-2.0 + y), -1 + (x + y));
	test_optimizer(__LINE__, (1.0 + x) + y, 1.0 + (x + y));
	test_optimizer(__LINE__, x + (1.0 + y), 1.0 + (x + y));
	test_optimizer(__LINE__, (x + 1.0) + (y + 2.0) + (z + 3.0), 6.0 + (x + y + z));
	
	printf("\noptimize[mult]:\n");
	test_optimizer(__LINE__, MathFunction(2.0) * 3.0, 6.0);
	test_optimizer(__LINE__, 0.0 * x, 0.0);
	test_optimizer(__LINE__, 1.0 * x, x);
	test_optimizer(__LINE__, (-1.0) * x, -x);
	test_optimizer(__LINE__, (-1.0) * (-x), x);
	test_optimizer(__LINE__, 2.0 * x, 2.0 * x);
	test_optimizer(__LINE__, 2.0 * (-x), (-2.0) * x);
	test_optimizer(__LINE__, 2.0 * (3.0 * x), 6.0 * x);
	test_optimizer(__LINE__, 2.0 * (0.0 * x), 0.0);
	test_optimizer(__LINE__, 2.0 * (0.5 * x), x);
	test_optimizer(__LINE__, 2.0 * (0.5 * (-x)), -x);
	test_optimizer(__LINE__, 2.0 * ((-0.5) * x), -x);
	test_optimizer(__LINE__, 2.0 * ((-0.5) * (-x)), x);
	test_optimizer(__LINE__, 2.0 * (3.0 * (-x)), (-6.0) * x);
	printf("\n");
	test_optimizer(__LINE__, x * 0.0, 0.0);
	test_optimizer(__LINE__, x * 1.0, x);
	test_optimizer(__LINE__, x * (-1.0), -x);
	test_optimizer(__LINE__, (-x) * (-1.0), x);
	test_optimizer(__LINE__, x * 2.0, 2.0 * x);
	test_optimizer(__LINE__, (-x) * 2.0, (-2.0) * x);
	test_optimizer(__LINE__, (x * 2.0) * 3.0, 6.0 * x);
	test_optimizer(__LINE__, (x * 0.5) * 2.0, x);
	test_optimizer(__LINE__, (x * (-0.5)) * 2.0, -x);
	test_optimizer(__LINE__, (x * (-0.5)) * (-2.0), x);
	test_optimizer(__LINE__, ((-x) * 2.0) * 3.0, (-6.0) * x);
	printf("\n");
	test_optimizer(__LINE__, (x * 2.0) * (y * 3.0), 6.0 * (x * y));
	test_optimizer(__LINE__, (x * 2.0) * (y * 0.5), x * y);
	test_optimizer(__LINE__, (x * (-2.0)) * (y * 0.5), - (x * y));
	test_optimizer(__LINE__, (x * 2.0) * y, 2.0 * (x * y));
	test_optimizer(__LINE__, (x * 2.0) * (-y), (-2.0) * (x * y));
	test_optimizer(__LINE__, x * (y * 2.0), 2.0 * (x * y));
	test_optimizer(__LINE__, (-x) * (y * 2.0), (-2.0) * (x * y));
	printf("\n");
	test_optimizer(__LINE__, x * y, x * y);
	test_optimizer(__LINE__, x * (-y), -(x * y));
	test_optimizer(__LINE__, (-x) * y, -(x * y));
	test_optimizer(__LINE__, (-x) * (-y), x * y);
	
	printf("\noptimize[temp sub/div]:\n");
	test_optimizer(__LINE__, 1.0 - (2.0 + x), (-1.0) - x);
	test_optimizer(__LINE__, x - (y + z), x - (y + z));
	test_optimizer(__LINE__, x + y - z, x + y - z);
	test_optimizer(__LINE__, 1.0 / (2.0 * x), 0.5 / x);
	test_optimizer(__LINE__, x / (y * z), x / (y * z));
	test_optimizer(__LINE__, x * y / z, x * y / z);
	test_optimizer(__LINE__, -(-x), x);
	test_optimizer(__LINE__, - sin(-x), sin(x));
	test_optimizer(__LINE__, cos(-x), cos(x));
	test_optimizer(__LINE__, inv(inv(x)), x);
	test_optimizer(__LINE__, ln(inv(x)), -ln(x));
	test_optimizer(__LINE__, -ln(inv(x)), ln(x));
	test_optimizer(__LINE__, (-x) / (-y), x / y);
	test_optimizer(__LINE__, x + y / (-z), x - (y / z));
	test_optimizer(__LINE__, x * inv(y), x / y);
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
	
	printf("\nPASSED: %d of %d\n", passed, count);
	if ( count == passed ) printf("OK\n");
	return (count == passed) ? 0 : 1;
}
