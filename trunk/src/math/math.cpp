
#include <math.h>
#include <nanosoft/math.h>

#include <string>
#include <stdio.h>

namespace nanosoft
{
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
		* Вычисление функции
		*/
		double eval() { return c; }
		
		/**
		* Вернуть производную функции
		*/
		MathFunctionImpl* derive(const MathVar &var) { return new MathConst(0); }
		
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
	* Конструктор переменной
	*/
	MathVar::MathVar(const char *n)
	{
		name = n;
	}
	
	/**
	* Вернуть название переменной
	*/
	const char * MathVar::getName()
	{
		return name;
	}
	
	/**
	* Вернуть значение переменной
	*/
	double MathVar::getValue()
	{
		return value;
	}
	
	/**
	* Установить значение переменной
	*/
	void MathVar::setValue(double v)
	{
		value = v;
	}
	
	/**
	* Вычислить значение переменной
	*/
	double MathVar::eval()
	{
		return value;
	}
	
	/**
	* Вернуть производную
	*/
	MathFunctionImpl* MathVar::derive(const MathVar &var)
	{
		return new MathConst(&var == this ? 1 : 0);
	}
	
	/**
	* Вернуть в виде строки
	*/
	std::string MathVar::toString()
	{
		return name;
	}
	
	/**
	* Функция f(x) = -x
	*/
	class MathNeg: public MathFunctionImpl
	{
	private:
		MathFunction a;
	public:
		MathNeg(MathFunction A): a(A) { }
		double eval() { return - a.eval(); }
		MathFunctionImpl* derive(const MathVar &var) { return new MathNeg(a.derive(var)); }
		
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
		MathSum(const MathFunction A, const MathFunction B): a(A), b(B) { }
		double eval() { return a.eval() + b.eval(); }
		MathFunctionImpl* derive(const MathVar &var) {
			return new MathSum(a.derive(var), b.derive(var));
		}
		
		/**
		* Вернуть в виде строки
		*/
		std::string toString() {
			return a.toString() + " + " + b.toString();
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
		MathMult(const MathFunction A, const MathFunction B): a(A), b(B) { }
		double eval() { return a.eval() * b.eval(); }
		MathFunctionImpl* derive(const MathVar &var) {
			return new MathSum(
				new MathMult(a.derive(var), b),
				new MathMult(a, b.derive(var))
			);
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
		MathCos(const MathFunction A): a(A) { }
		double eval() { return ::cos(a.eval()); }
		MathFunctionImpl* derive(const MathVar &var);
		
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
		MathSin(const MathFunction A): a(A) { }
		double eval() { return ::sin(a.eval()); }
		MathFunctionImpl* derive(const MathVar &var);
		
		/**
		* Вернуть в виде строки
		*/
		std::string toString() {
			return "sin(" + a.toString() + ")";
		}
	};
	
	/**
	* Производная sin(x)
	*/
	MathFunctionImpl* MathSin::derive(const MathVar &var)
	{
		return new MathMult(new MathCos(a), a.derive(var));
	}
	
	/**
	* Производная cos(x)
	*/
	MathFunctionImpl* MathCos::derive(const MathVar &var)
	{
		return new MathNeg(
			new MathMult(new MathSin(a), a.derive(var))
		);
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
	* Сумма функций
	*/
	MathFunction operator + (MathFunction a, MathFunction b)
	{
		return new MathSum(a, b);
	}
	
	/**
	* Сумма функции и константы
	*/
	MathFunction operator + (MathFunction a, double b)
	{
		return new MathSum(a, MathFunction(b));
	}
	
	/**
	* Сумма функции и константы
	*/
	MathFunction operator + (double a, MathFunction b)
	{
		return new MathSum(MathFunction(a), b);
	}
	
	/**
	* Произведение функций
	*/
	MathFunction operator * (MathFunction a, MathFunction b)
	{
		return new MathMult(a, b);
	}
	
	/**
	* Произведение функции и константы
	*/
	MathFunction operator * (MathFunction a, double b)
	{
		return new MathMult(a, MathFunction(b));
	}
	
	/**
	* Произведение функции и константы
	*/
	MathFunction operator * (double a, MathFunction b)
	{
		return new MathMult(MathFunction(a), b);
	}
	
	/**
	* Функция sin(x)
	*/
	MathFunction sin(MathFunction x)
	{
		return new MathSin(x);
	}
	
	/**
	* Функция cos(x)
	*/
	MathFunction cos(MathFunction x)
	{
		return new MathCos(x);
	}
}
