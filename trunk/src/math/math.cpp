
#include <math.h>
#include <nanosoft/math.h>

#include <string>
#include <stdio.h>
#include <iostream>

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
		* Вернуть в виде строки
		*/
		std::string toString() {
			char buf[80];
			int len = sprintf(buf, "%f", c);
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
		* Вернуть в виде строки
		*/
		std::string toString();
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
	* Вернуть оптимизированную функцию
	*/
	MathFunction MathFunctionImpl::optimize()
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
		var->lock();
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
	* Вернуть функцию f(x) = x
	*/
	MathVar::operator MathFunction ()
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
		MathFunction optimize() {
			MathFunction x = a.optimize();
			if ( a.getType() == "const" ) return -a.eval();
			MathNeg *xn = x.cast<MathNeg>();
			return xn ? xn->a : (-x);
		}
		
		/**
		* Вернуть в виде строки
		*/
		std::string toString() {
			return "-(" + a.toString() + ")";
		}
	};
	
	/**
	* Функция F(x,y) = x + y
	*/
	class MathSum: public MathFunctionImpl
	{
	private:
		MathFunction a;
		MathFunction b;
	public:
		MathSum(const MathFunction &A, const MathFunction &B): a(A), b(B) { }
		std::string getType() { return "sum"; }
		double eval() { return a.eval() + b.eval(); }
		MathFunction derive(const MathVar &var) { return a.derive(var) + b.derive(var); }
		MathFunction optimize() {
			MathFunction x = a.optimize();
			MathFunction y = b.optimize();
			std::cout << "x.getType() = " << x.getType() << ", y.getType() = " << y.getType() << std::endl;
			if ( x.getType() == "const" )
			{
				if ( x.eval() == 0.0 ) return y;
				if ( y.getType() == "const" ) return x.eval() + y.eval();
				MathSum *ys = y.cast<MathSum>();
				if ( ys && ys->b.getType() == "const" )
				{
					double c = x.eval() + ys->b.eval();
					return c == 0.0 ? ys->a : (ys->a + c);
				}
				MathNeg *yn = y.cast<MathNeg>();
				return yn ? (x - yn->a) : (y + x);
			}
			if ( y.getType() == "const" )
			{
				MathSum *xs = x.cast<MathSum>();
				if ( xs && xs->b.getType() == "const" )
				{
					double c = y.eval() + xs->b.eval();
					return c == 0.0 ? xs->a : (xs->a + c);
				}
				if ( y.eval() == 0.0 ) return x;
				return x + y;
			}
			
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
	};
	
	/**
	* Функция F(x,y) = x - y
	*/
	class MathSub: public MathFunctionImpl
	{
	private:
		MathFunction a;
		MathFunction b;
	public:
		MathSub(const MathFunction &A, const MathFunction &B): a(A), b(B) { }
		std::string getType() { return "sub"; }
		double eval() { return a.eval() - b.eval(); }
		MathFunction derive(const MathVar &var) { return a.derive(var) - b.derive(var); }
		
		/**
		* Вернуть в виде строки
		*/
		std::string toString() {
			return a.toString() + " - " + b.toString();
		}
	};
	
	/**
	* Функция F(x,y) = x * y
	*/
	class MathMult: public MathFunctionImpl
	{
	private:
		MathFunction a;
		MathFunction b;
	public:
		MathMult(const MathFunction &A, const MathFunction &B): a(A), b(B) { }
		std::string getType() { return "mult"; }
		double eval() { return a.eval() * b.eval(); }
		MathFunction derive(const MathVar &var) { return a.derive(var) * b + a * b.derive(var); }
		MathFunction optimize() {
			MathFunction x = a.optimize();
			MathFunction y = b.optimize();
			if ( x.getType() == "const" )
			{
				if ( x.eval() == 0.0 ) return 0.0;
				if ( x.eval() == 1.0 ) return y;
				if ( y.getType() == "const" ) return x.eval() * y.eval();
				
				MathMult *ym = y.cast<MathMult>();
				if ( ym && ym->b.getType() == "const" )
				{
					double c = x.eval() * ym->b.eval();
					if ( c == 0.0 ) return 0.0;
					if ( c == 1.0 ) return ym->a;
					return ym->a * c;
				}
				
				MathNeg *yn = y.cast<MathNeg>();
				if ( yn ) return yn->a * (-x.eval());
				
				return y * x;
			}
			if ( y.getType() == "const" )
			{
				if ( y.eval() == 0.0 ) return 0.0;
				if ( y.eval() == 1.0 ) return x;
				
				MathMult *xm = x.cast<MathMult>();
				if ( xm && xm->b.getType() == "const" )
				{
					double c = y.eval() * xm->b.eval();
					if ( c == 0.0 ) return 0.0;
					if ( c == 1.0 ) return xm->a;
					return xm->a * c;
				}
				
				MathNeg *xn = x.cast<MathNeg>();
				if ( xn ) return xn->a * (-y.eval());
				
				return x * y;
			}
			
			MathNeg *xn = x.cast<MathNeg>();
			MathNeg *yn = y.cast<MathNeg>();
			if ( xn )
			{
				if ( yn ) return xn->a * yn->a;
				return - (xn->a * y);
			}
			if ( yn ) return - (x * yn->a);
			
			return x * y;
		}
		
		/**
		* Вернуть в виде строки
		*/
		std::string toString() {
			return "(" + a.toString() + ") * (" + b.toString() + ")";
		}
	};
	
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
		
		/**
		* Вернуть в виде строки
		*/
		std::string toString() {
			return "cos(" + a.toString() + ")";
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
		
		/**
		* Вернуть в виде строки
		*/
		std::string toString() {
			return "sin(" + a.toString() + ")";
		}
	};
	
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
}
