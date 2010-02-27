
#include <math.h>
#include <nanosoft/math.h>

#include <string>
#include <stdio.h>

namespace nanosoft
{
	/**
	* Функция-константа
	*/
	class MathConst: public MathFunctionImpl
	{
	private:
		/**
		* Значение константы
		*/
		double c;
	public:
		/**
		* Конструктор
		*/
		MathConst(double value): c(value) { }
		
		/**
		* Вернуть тип функции
		*/
		std::string getType() { return "const"; }
		
		/**
		* Вычисление функции
		*/
		double eval() { return c; }
		
		/**
		* Вернуть производную функции
		*/
		MathFunction derive(const MathVar &var) { return new MathConst(0); }
		
		/**
		* Первый этап оптимизации (константы, суммы, произведения)
		*/
		MathFunction optimize1() { return this; }
		
		/**
		* Второй этап оптимизации (светывание разностей и частных)
		*/
		MathFunction optimize2() { return this; }
		
		/**
		* Вернуть в виде строки
		*/
		std::string toString() {
			char buf[80];
			int len = sprintf(buf, "%f", c);
			return std::string(buf, len);
		}
		
		/**
		* Вернуть в виде строки для отладки и тестирования
		*/
		std::string debugString() {
			char buf[80];
			int len = sprintf(buf, "%.2f", c);
			return std::string(buf, len);
		}
	};
	
	/**
	* Класс переменной
	*/
	class MathVarImpl: public MathFunctionImpl
	{
	private:
		/**
		* Имя переменной
		*/
		const char *name;
		
		/**
		* Значение переменной
		*/
		double value;
	public:
		/**
		* Конструктор переменной
		*/
		MathVarImpl(const char *n, double v);
		
		/**
		* Деструктор переменной
		*/
		~MathVarImpl();
		
		/**
		* Вернуть тип функции
		*/
		std::string getType() { return "var"; }
		
		/**
		* Вернуть название переменной
		*/
		const char *getName();
		
		/**
		* Вернуть значение переменной
		*/
		double getValue();
		
		/**
		* Установить значение переменной
		*/
		void setValue(double v);
		
		/**
		* Вычислить значение переменной
		*/
		double eval();
		
		/**
		* Вернуть производную
		*/
		MathFunction derive(const MathVar &var);
		
		/**
		* Первый этап оптимизации (константы, суммы, произведения)
		*/
		MathFunction optimize1() { return this; }
		
		/**
		* Второй этап оптимизации (светывание разностей и частных)
		*/
		MathFunction optimize2() { return this; }
		
		/**
		* Вернуть в виде строки
		*/
		std::string toString();
		
		/**
		* Вернуть в виде строки для отладки и тестирования
		*/
		std::string debugString();
	};
	
	/**
	* Конструктор переменной
	*/
	MathVarImpl::MathVarImpl(const char *n, double v)
	{
		name = n;
		value = v;
	}
	
	/**
	* Деструктор переменной
	*/
	MathVarImpl::~MathVarImpl()
	{
	}
	
	/**
	* Вернуть название переменной
	*/
	const char * MathVarImpl::getName()
	{
		return name;
	}
	
	/**
	* Вернуть значение переменной
	*/
	double MathVarImpl::getValue()
	{
		return value;
	}
	
	/**
	* Установить значение переменной
	*/
	void MathVarImpl::setValue(double v)
	{
		value = v;
	}
	
	/**
	* Вычислить значение переменной
	*/
	double MathVarImpl::eval()
	{
		return value;
	}
	
	/**
	* Вернуть производную
	*/
	MathFunction MathVarImpl::derive(const MathVar &var)
	{
		return new MathConst(var == this ? 1 : 0);
	}
	
	/**
	* Вернуть в виде строки
	*/
	std::string MathVarImpl::toString()
	{
		return name;
	}
	
	/**
	* Вернуть в виде строки для отладки и тестирования
	*/
	std::string MathVarImpl::debugString()
	{
		return name;
	}
	
	/**
	* Конструктор по умолчанию
	*/
	MathFunctionImpl::MathFunctionImpl()
	{
		ref_count = 0;
	}
	
	/**
	* Виртуальный деструктор
	*/
	MathFunctionImpl::~MathFunctionImpl()
	{
	}
	
	/**
	* Первый этап оптимизации (константы, суммы, произведения)
	*/
	MathFunction MathFunctionImpl::optimize1()
	{
		// default optimization: no optimization
		return this;
	}
	
	/**
	* Увеличить счетчик ссылок
	*/
	void MathFunctionImpl::lock()
	{
		ref_count++;
	}
	
	/**
	* Уменьшить счетчик ссылок, если станет 0, то самоудалиться
	*/
	void MathFunctionImpl::release()
	{
		ref_count--;
		if ( ref_count == 0 ) delete this;
	}
	
	/**
	* Конструктор
	*/
	MathVar::MathVar()
	{
		var = 0;
	}
	
	/**
	* Конструктор
	*/
	MathVar::MathVar(const char *name, double value)
	{
		var = new MathVarImpl(name, value);
		var->lock();
	}
	
	/**
	* Конструктор копий
	*/
	MathVar::MathVar(const MathVar &v)
	{
		var = v.var;
		if ( var ) var->lock();
	}
	
	/**
	* Деструктор
	*/
	MathVar::~MathVar()
	{
		if ( var ) var->release();
	}
	
	/**
	* Вернуть название переменной
	*/
	const char * MathVar::getName() const
	{
		return var->getName();
	}
	
	/**
	* Вернуть значение переменной
	*/
	double MathVar::getValue() const
	{
		return var->getValue();
	}
	
	/**
	* Установить значение переменной
	*/
	void MathVar::setValue(double v)
	{
		var->setValue(v);
	}
	
	/**
	* Оператор копирования переменной
	*/
	void MathVar::operator = (const MathVar &a)
	{
		if ( var ) var->release();
		var = a.var;
		if ( var ) var->lock();
	}
	
	/**
	* Вернуть функцию f(x) = x
	*/
	MathVar::operator MathFunction () const
	{
		return MathFunction(var);
	}
	
	/**
	* Функция f(x) = -x
	*/
	class MathNeg: public MathFunctionImpl
	{
	public:
		MathFunction a;
		MathNeg(MathFunction A): a(A) { }
		std::string getType() { return "neg"; }
		double eval() { return - a.eval(); }
		MathFunction derive(const MathVar &var) { return new MathNeg(a.derive(var)); }
		MathFunction optimize1();
		MathFunction optimize2() { return - a.optimize2(); }
		
		/**
		* Вернуть в виде строки
		*/
		std::string toString() {
			return "-(" + a.toString() + ")";
		}
		
		/**
		* Вернуть в виде строки для отладки и тестирования
		*/
		std::string debugString() {
			return "neg(" + a.debugString() + ")";
		}
	};
	
	/**
	* Функция F(x,y) = x + y
	*/
	class MathSum: public MathFunctionImpl
	{
	public:
		MathFunction a;
		MathFunction b;
		MathSum(const MathFunction &A, const MathFunction &B): a(A), b(B) { }
		std::string getType() { return "sum"; }
		double eval() { return a.eval() + b.eval(); }
		MathFunction derive(const MathVar &var) { return a.derive(var) + b.derive(var); }
		MathFunction optimize1() {
			MathFunction x = a.optimize1();
			MathFunction y = b.optimize1();
			
			MathSum *xs = x.cast<MathSum>();
			MathSum *ys = y.cast<MathSum>();
			
			if ( x.getType() == "const" )
			{
				if ( x.eval() == 0.0 ) return y;
				if ( y.getType() == "const" ) return x.eval() + y.eval();
				if ( ys && ys->a.getType() == "const" )
				{
					double c = x.eval() + ys->a.eval();
					if ( c == 0.0 ) return ys->b;
					return c + ys->b;
				}
				return x + y;
			}
			
			if ( y.getType() == "const" )
			{
				if ( y.eval() == 0.0 ) return x;
				if ( xs && xs->a.getType() == "const" )
				{
					double c = y.eval() + xs->a.eval();
					if ( c == 0.0 ) return xs->b;
					return c + xs->b;
				}
				return y + x;
			}
			
			if ( xs && xs->a.getType() == "const" )
			{
				if ( ys && ys->a.getType() == "const" )
				{
					double c = xs->a.eval() + ys->a.eval();
					if ( c == 0.0 ) return xs->b + ys->b;
					return c + (xs->b + ys->b);
				}
				return xs->a.eval() + (xs->b + y);
			}
			
			if ( ys && ys->a.getType() == "const" )
			{
				return ys->a.eval() + (x + ys->b);
			}
			
			return x + y;
		}
		
		MathFunction optimize2() {
			MathFunction x = a.optimize2();
			MathFunction y = b.optimize2();
			
			MathNeg *xn = x.cast<MathNeg>();
			MathNeg *yn = y.cast<MathNeg>();
			
			if ( xn )
			{
				if ( yn ) return -(xn->a + yn->a);
				return y - xn->a;
			}
			
			if ( yn ) return x - yn->a;
			
			return x + y;
		}
		
		/**
		* Вернуть в виде строки
		*/
		std::string toString() {
			return a.toString() + " + " + b.toString();
		}
		
		/**
		* Вернуть в виде строки для отладки и тестирования
		*/
		std::string debugString() {
			return "(" + a.debugString() + " + " + b.debugString() + ")";
		}
	};
	
	/**
	* Функция F(x,y) = x - y
	*/
	class MathSub: public MathFunctionImpl
	{
	public:
		MathFunction a;
		MathFunction b;
		MathSub(const MathFunction &A, const MathFunction &B): a(A), b(B) { }
		std::string getType() { return "sub"; }
		double eval() { return a.eval() - b.eval(); }
		MathFunction derive(const MathVar &var) { return a.derive(var) - b.derive(var); }
		MathFunction optimize1() {
			return (a + (-b)).optimize1();
		}
		MathFunction optimize2() { return a.optimize2() - b.optimize2(); }
		
		/**
		* Вернуть в виде строки
		*/
		std::string toString() {
			return a.toString() + " - " + b.toString();
		}
		
		/**
		* Вернуть в виде строки для отладки и тестирования
		*/
		std::string debugString() {
			return "(" + a.debugString() + " - " + b.debugString() + ")";
		}
	};
	
	MathFunction MathNeg::optimize1()
	{
		MathSum *sum = a.cast<MathSum>();
		if ( sum ) return (- sum->a - sum->b).optimize1();
		
		MathSub *sub = a.cast<MathSub>();
		if ( sub ) return (- sub->a + sub->b).optimize1();
		
		MathFunction x = a.optimize1();
		
		MathConst *c = x.cast<MathConst>();
		if ( c ) return (-c->eval());
		
		MathNeg *neg = x.cast<MathNeg>();
		if ( neg ) return neg->a.optimize1();
		
		return - x;
	}
	
	/**
	* Функция f(x) = 1/x
	*/
	class MathInv: public MathFunctionImpl
	{
	public:
		MathFunction a;
		MathInv(const MathFunction &A): a(A) { }
		std::string getType() { return "inv"; }
		double eval() { return 1.0 / a.eval(); }
		MathFunction derive(const MathVar &var) { return - (pow(a, -2.0) * a.derive(var)); }
		MathFunction optimize1();
		MathFunction optimize2() { return inv(a.optimize()); }
		
		/**
		* Вернуть в виде строки
		*/
		std::string toString() {
			return "inv(" + a.toString() + ")";
		}
		
		/**
		* Вернуть в виде строки для отладки и тестирования
		*/
		std::string debugString() {
			return "inv(" + a.debugString() + ")";
		}
	};
	
	/**
	* Функция F(x,y) = x * y
	*/
	class MathMult: public MathFunctionImpl
	{
	public:
		MathFunction a;
		MathFunction b;
		MathMult(const MathFunction &A, const MathFunction &B): a(A), b(B) { }
		std::string getType() { return "mult"; }
		double eval() { return a.eval() * b.eval(); }
		MathFunction derive(const MathVar &var) { return a.derive(var) * b + a * b.derive(var); }
		MathFunction optimize1() {
			MathFunction x = a.optimize1();
			MathFunction y = b.optimize1();
			
			MathMult *xm = x.cast<MathMult>();
			MathMult *ym = y.cast<MathMult>();
			MathNeg *xn = x.cast<MathNeg>();
			MathNeg *yn = y.cast<MathNeg>();
			
			if ( x.getType() == "const" )
			{
				if ( x.eval() == 0.0 ) return 0.0;
				if ( x.eval() == 1.0 ) return y;
				if ( x.eval() == -1.0 ) return yn ? yn->a : (-y);
				if ( y.getType() == "const" ) return x.eval() * y.eval();
				if ( yn ) return (-x.eval()) * yn->a;
				if ( ym && ym->a.getType() == "const" )
				{
					double c = x.eval() * ym->a.eval();
					if ( c == 1.0 ) return ym->b;
					if ( c == -1.0 ) return - ym->b;
					return c * ym->b;
				}
				return x * y;
			}
			
			if ( y.getType() == "const" )
			{
				if ( y.eval() == 0.0 ) return 0.0;
				if ( y.eval() == 1.0 ) return x;
				if ( y.eval() == -1.0 ) return xn ? xn->a : (-x);
				if ( xn ) return (-y.eval()) * xn->a;
				if ( xm && xm->a.getType() == "const" )
				{
					double c = y.eval() * xm->a.eval();
					if ( c == 0.0 ) return 0.0;
					if ( c == 1.0 ) return xm->b;
					if ( c == -1.0 ) return -xm->b;
					return c * xm->b;
				}
				return y * x;
			}
			
			if ( xm && xm->a.getType() == "const" )
			{
				if ( ym && ym->a.getType() == "const" )
				{
					double c = xm->a.eval() * ym->a.eval();
					if ( c == 1.0 ) return xm->b * ym->b;
					if ( c == -1.0 ) return -(xm->b * ym->b);
					return c * (xm->b * ym->b);
				}
				if ( yn ) return (-xm->a.eval()) * (xm->b * yn->a);
				return xm->a.eval() * (xm->b * y);
			}
			
			if ( ym && ym->a.getType() == "const" )
			{
				if ( xn ) return (-ym->a.eval()) * (xn->a * ym->b);
				return ym->a.eval() * (x * ym->b);
			}
			
			if ( xn )
			{
				if ( yn ) return xn->a * yn->a;
				return - (xn->a * y);
			}
			if ( yn ) return - (x * yn->a);
			
			return x * y;
		}
		
		MathFunction optimize2() {
			MathFunction x = a.optimize2();
			MathFunction y = b.optimize2();
			
			MathInv *xi = x.cast<MathInv>();
			MathInv *yi = y.cast<MathInv>();
			
			if ( xi )
			{
				if ( yi ) return inv(xi->a * yi->a);
				return y / xi->a;
			}
			
			if ( yi ) return x / yi->a;
			
			return x * y;
		}
		
		/**
		* Вернуть в виде строки
		*/
		std::string toString() {
			return "(" + a.toString() + ") * (" + b.toString() + ")";
		}
		
		/**
		* Вернуть в виде строки для отладки и тестирования
		*/
		std::string debugString() {
			return "(" + a.debugString() + " * " + b.debugString() + ")";
		}
	};
	
	/**
	* Функция F(x,y) = x / y
	*/
	class MathDiv: public MathFunctionImpl
	{
	public:
		MathFunction a;
		MathFunction b;
		MathDiv(const MathFunction &A, const MathFunction &B): a(A), b(B) { }
		std::string getType() { return "div"; }
		double eval() { return a.eval() / b.eval(); }
		MathFunction derive(const MathVar &var) {
			// TODO b^2
			return (a.derive(var) * b - a * b.derive(var)) / pow(b, 2);
		}
		MathFunction optimize1() {
			return (a * inv(b)).optimize1();
		}
		MathFunction optimize2() { return a.optimize2() / b.optimize2(); }
		
		/**
		* Вернуть в виде строки
		*/
		std::string toString() {
			return "(" + a.toString() + ") / (" + b.toString() + ")";
		}
		
		/**
		* Вернуть в виде строки для отладки и тестирования
		*/
		std::string debugString() {
			return "(" + a.debugString() + " / " + b.debugString() + ")";
		}
	};
	
	MathFunction MathInv::optimize1()
	{
		MathNeg *neg = a.cast<MathNeg>();
		if ( neg ) return (-inv(neg->a)).optimize1();
		
		MathMult *mult = a.cast<MathMult>();
		if ( mult ) return (inv(mult->a) * inv(mult->b)).optimize1();
		
		MathDiv *div = a.cast<MathDiv>();
		if ( div ) return (inv(mult->a) * mult->b).optimize1();
		
		MathFunction x = a.optimize1();
		
		MathConst *c = x.cast<MathConst>();
		if ( c ) return 1 / c->eval();
		
		MathInv *inv = x.cast<MathInv>();
		if ( inv ) return inv->a.optimize1();
		
		return nanosoft::inv(x);
	}
	
	/**
	* Первый этап оптимизатора по умолчанию для функции одной переменной
	*/
	MathFunction opt1_default(MathFunctionX f, const MathFunction &a)
	{
		MathFunction x = a.optimize1();
		if ( x.getType() == "const" ) return f(x).eval();
		return f(x);
	}
	
	/**
	* Второй этап оптимизатора по умолчанию для функции одной переменной
	*/
	MathFunction opt2_default(MathFunctionX f, const MathFunction &a)
	{
		return f(a.optimize2());
	}
	
	/**
	* Первый этап оптимизатора по умолчанию для функции двух переменных
	*/
	MathFunction opt1_default(MathFunctionXY f, const MathFunction &a, const MathFunction &b)
	{
		MathFunction x = a.optimize1();
		MathFunction y = b.optimize1();
		if ( x.getType() == "const" && x.getType() == "const" ) return f(x, y).eval();
		return f(x, y);
	}
	
	/**
	* Втрой этап оптимизатора по умолчанию для функции двух переменных
	*/
	MathFunction opt2_default(MathFunctionXY f, const MathFunction &a, const MathFunction &b)
	{
		return f(a.optimize2(), b.optimize2());
	}
	
	/**
	* Оптимизатор четной функции
	*/
	MathFunction opt1_even(MathFunctionX f, const MathFunction &a)
	{
		MathFunction x = a.optimize1();
		if ( x.getType() == "const" ) return f(x).eval();
		MathNeg *nx = x.cast<MathNeg>();
		return f( nx ? nx->a : x);
	}
	
	/**
	* Оптимизатор нечетной функции
	*/
	MathFunction opt1_odd(MathFunctionX f, const MathFunction &a)
	{
		MathFunction x = a.optimize1();
		if ( x.getType() == "const" ) return f(x).eval();
		MathNeg *nx = x.cast<MathNeg>();
		if ( nx ) return - f(nx->a);
		return f(x);
	}
	
	/**
	* Функция F(x) = cos(x)
	*/
	class MathCos: public MathFunctionImpl
	{
	private:
		MathFunction a;
	public:
		MathCos(const MathFunction &A): a(A) { }
		std::string getType() { return "cos"; }
		double eval() { return ::cos(a.eval()); }
		MathFunction derive(const MathVar &var) { return - sin(a) * a.derive(var); }
		MathFunction optimize1() { return opt1_even(cos, a); }
		MathFunction optimize2() { return opt2_default(cos, a); }
		
		/**
		* Вернуть в виде строки
		*/
		std::string toString() {
			return "cos(" + a.toString() + ")";
		}
		
		/**
		* Вернуть в виде строки для отладки и тестирования
		*/
		std::string debugString() {
			return "cos(" + a.debugString() + ")";
		}
	};
	
	/**
	* Функция F(x) = sin(x)
	*/
	class MathSin: public MathFunctionImpl
	{
	private:
		MathFunction a;
	public:
		MathSin(const MathFunction &A): a(A) { }
		std::string getType() { return "sin"; }
		double eval() { return ::sin(a.eval()); }
		MathFunction derive(const MathVar &var) { return cos(a) * a.derive(var); }
		MathFunction optimize1() { return opt1_odd(sin, a); }
		MathFunction optimize2() { return opt2_default(sin, a); }
		
		/**
		* Вернуть в виде строки
		*/
		std::string toString() {
			return "sin(" + a.toString() + ")";
		}
		
		/**
		* Вернуть в виде строки для отладки и тестирования
		*/
		std::string debugString() {
			return "sin(" + a.debugString() + ")";
		}
	};
	
	/**
	* Функция F(x) = exp(x)
	*/
	class MathExp: public MathFunctionImpl
	{
	private:
		MathFunction a;
	public:
		MathExp(const MathFunction &A): a(A) { }
		std::string getType() { return "exp"; }
		double eval() { return ::exp(a.eval()); }
		MathFunction derive(const MathVar &var) { return exp(a) * a.derive(var); }
		MathFunction optimize1() { return opt1_default(exp, a); }
		MathFunction optimize2() { return opt2_default(exp, a); }
		
		/**
		* Вернуть в виде строки
		*/
		std::string toString() {
			return "exp(" + a.toString() + ")";
		}
		
		/**
		* Вернуть в виде строки для отладки и тестирования
		*/
		std::string debugString() {
			return "exp(" + a.debugString() + ")";
		}
	};
	
	/**
	* Функция F(x) = ln(x) (натуральный логарифм)
	*/
	class MathLn: public MathFunctionImpl
	{
	private:
		MathFunction a;
	public:
		MathLn(const MathFunction &A): a(A) { }
		std::string getType() { return "ln"; }
		double eval() { return ::log(a.eval()); }
		MathFunction derive(const MathVar &var) { return a.derive(var) / a; }
		MathFunction optimize1() {
			MathFunction x = a.optimize1();
			if ( x.getType() == "const" ) return ln(x).eval();
			MathInv *xi = x.cast<MathInv>();
			if ( xi ) return -ln(xi->a);
			return ln(x);
		}
		MathFunction optimize2() { return opt2_default(ln, a); }
		
		/**
		* Вернуть в виде строки
		*/
		std::string toString() {
			return "ln(" + a.toString() + ")";
		}
		
		/**
		* Вернуть в виде строки для отладки и тестирования
		*/
		std::string debugString() {
			return "ln(" + a.debugString() + ")";
		}
	};
	
	/**
	* Функция F(x,y) = x^y
	*/
	class MathPow: public MathFunctionImpl
	{
	private:
		MathFunction a;
		MathFunction b;
	public:
		MathPow(const MathFunction &A, const MathFunction &B): a(A), b(B) { }
		std::string getType() { return "pow"; }
		double eval() { return ::pow(a.eval(), b.eval()); }
		MathFunction derive(const MathVar &var) {
			return b * pow(a, b-1) * a.derive(var) + pow(a, b) * ln(a) * b.derive(var);
		}
		MathFunction optimize1() { return opt1_default(pow, a, b); }
		MathFunction optimize2() { return opt2_default(pow, a, b); }
		
		/**
		* Вернуть в виде строки
		*/
		std::string toString() {
			return "(" + a.toString() + ")^(" + b.toString() + ")";
		}
		
		/**
		* Вернуть в виде строки для отладки и тестирования
		*/
		std::string debugString() {
			return "(" + a.debugString() + " ^ " + b.debugString() + ")";
		}
	};
	
	/**
	* Конструктор по умолчанию (нуль-функция)
	*/
	MathFunction::MathFunction()
	{
		func = new MathConst(0);
		func->lock();
	}
	
	/**
	* Конструктор константной функции
	*/
	MathFunction::MathFunction(double c)
	{
		func = new MathConst(c);
		func->lock();
	}
	
	/**
	* Конструктор функции
	*/
	MathFunction::MathFunction(MathFunctionImpl *impl)
	{
		func = impl;
		if ( func ) func->lock();
	}
	
	/**
	* Конструктор копии
	*/
	MathFunction::MathFunction(const MathFunction &f)
	{
		func = f.func;
		if ( func ) func->lock();
	}
	
	/**
	* Деструктор функции
	*/
	MathFunction::~MathFunction()
	{
		if ( func ) func->release();
	}
	
	/**
	* Вернуть тип функции
	*/
	std::string MathFunction::getType() const { return func->getType(); }
	
	/**
	* Вычислить функцию
	*/
	double MathFunction::eval() const { return func->eval(); }
	
	/**
	* Вернуть производную функции
	* @param var переменная по которой будет дифференцирование
	*/
	MathFunction MathFunction::derive(const MathVar &var) const { return func->derive(var); }
	
	/**
	* Вернуть оптимизированную функцию
	*/
	MathFunction MathFunction::optimize1() const
	{
		return func->optimize1();
	}
	
	/**
	* Вернуть оптимизированную функцию
	*/
	MathFunction MathFunction::optimize2() const
	{
		return func->optimize2();
	}
	
	/**
	* Вернуть оптимизированную функцию
	*/
	MathFunction MathFunction::optimize() const
	{
		return optimize1().optimize2();
	}
	
	void MathFunction::operator = (const MathFunction &a)
	{
		if ( func ) func->release();
		func = a.func;
		func->lock();
	}
	
	/**
	* Вернуть функцию в виде строки
	*/
	std::string MathFunction::toString() const
	{
		return func->toString();
	}
	
	/**
	* Вернуть в виде строки для отладки и тестирования
	*/
	std::string MathFunction::debugString() const
	{
		return func->debugString();
	}
	
	/**
	* Унарный оператор вычитания
	*/
	MathFunction operator - (const MathFunction &a)
	{
		return new MathNeg(a);
	}
	
	/**
	* Сумма функций
	*/
	MathFunction operator + (const MathFunction &a, const MathFunction &b)
	{
		return new MathSum(a, b);
	}
	
	/**
	* Сумма функции и константы
	*/
	MathFunction operator + (const MathFunction &a, double b)
	{
		return new MathSum(a, b);
	}
	
	/**
	* Сумма функции и константы
	*/
	MathFunction operator + (double a, const MathFunction &b)
	{
		return new MathSum(a, b);
	}
	
	/**
	* Разность функций
	*/
	MathFunction operator - (const MathFunction &a, const MathFunction &b)
	{
		return new MathSub(a, b);
	}
	
	/**
	* Сумма функции и константы
	*/
	MathFunction operator - (const MathFunction &a, double b)
	{
		return new MathSub(a, b);
	}
	
	/**
	* Сумма функции и константы
	*/
	MathFunction operator - (double a, const MathFunction &b)
	{
		return new MathSub(a, b);
	}
	
	/**
	* Произведение функций
	*/
	MathFunction operator * (const MathFunction &a, const MathFunction &b)
	{
		return new MathMult(a, b);
	}
	
	/**
	* Произведение функции и константы
	*/
	MathFunction operator * (const MathFunction &a, double b)
	{
		return new MathMult(a, b);
	}
	
	/**
	* Произведение функции и константы
	*/
	MathFunction operator * (double a, const MathFunction &b)
	{
		return new MathMult(a, b);
	}
	
	/**
	* Деление функций
	*/
	MathFunction operator / (const MathFunction &a, const MathFunction &b)
	{
		return new MathDiv(a, b);
	}
	
	/**
	* Деление функции на константу
	*/
	MathFunction operator / (const MathFunction &a, double b)
	{
		return new MathDiv(a, b);
	}
	
	/**
	* Деление константы на функцию
	*/
	MathFunction operator / (double a, const MathFunction &b)
	{
		return new MathDiv(a, b);
	}
	
	/**
	* Функция inv(x) = 1 / x
	*/
	MathFunction inv(const MathFunction &a)
	{
		return new MathInv(a);
	}
	
	/**
	* Функция sin(x)
	*/
	MathFunction sin(const MathFunction &x)
	{
		return new MathSin(x);
	}
	
	/**
	* Функция cos(x)
	*/
	MathFunction cos(const MathFunction &x)
	{
		return new MathCos(x);
	}
	
	/**
	* Функция exp(x)
	*/
	MathFunction exp(const MathFunction &x)
	{
		return new MathExp(x);
	}
	
	/**
	* Функция pow(x, y) = x ^ y
	*/
	MathFunction pow(const MathFunction &x, const MathFunction &y)
	{
		return new MathPow(x, y);
	}
	
	/**
	* Функция ln(x) (натуальный логарифм)
	*/
	MathFunction ln(const MathFunction &x)
	{
		return new MathLn(x);
	}
	
	/**
	* Функция log(x, a) - логарифм числа x по основанию a
	*/
	MathFunction log(const MathFunction &x, const MathFunction &a)
	{
		return ln(x) / ln(a);
	}
	
	/**
	* Оптимизация функции
	*/
	MathFunction optimize(const MathFunction &f)
	{
		return f.optimize();
	}
	
	/**
	* Производная функции
	*/
	MathFunction derive(const MathFunction &f, const MathVar &var)
	{
		return f.derive(var);
	}
}
