#ifndef NANOSOFT_MATH_PARSER_H
#define NANOSOFT_MATH_PARSER_H

#include <math.h>
#include <string>
#include <stdio.h>

namespace nanosoft
{
	template <class type> class MathVar;
	
	/**
	* Абстрактный класс реализации метематической функции
	*/
	template <class type>
	class MathFunctionImpl
	{
	private:
		/**
		* Счетчик ссылок
		*/
		int ref_count;
	public:
		/**
		* Конструктор по умолчанию
		*/
		MathFunctionImpl(): ref_count(0) { }
		
		/**
		* Деструктор виртуальный
		*/
		virtual ~MathFunctionImpl() { }
		
		/**
		* Вычислить функцию
		*/
		virtual type eval() = 0;
		
		/**
		* Вернуть производную функции
		*/
		virtual MathFunctionImpl<type>* derive(const MathVar<type> &var) = 0;
		
		/**
		* Вернуть в виде строки
		*/
		virtual std::string toString() = 0;
		
		/**
		* Увеличить счетчик ссылок
		*/
		void lock() { ref_count++; }
		
		/**
		* Уменьшить счетчик ссылок, если станет 0, то самоудалиться
		*/
		void release() {
			ref_count--;
			if ( ref_count == 0 ) delete this;
		}
	};
	
	/**
	* Базовый класс математической функции
	*/
	template <class type>
	class MathFunction
	{
	private:
		/**
		* Реализация функции
		*/
		MathFunctionImpl<type> *func;
	public:
		/**
		* Конструктор константной функции
		*/
		MathFunction(type c);
		
		/**
		* Конструктор функции
		*/
		MathFunction(MathFunctionImpl<type> *impl) { func = impl; }
		
		/**
		* Вычислить функцию
		*/
		type eval() { return func->eval(); }
		
		/**
		* Вернуть производную функции
		*/
		MathFunction<type> derive(const MathVar<type> &var) { return func->derive(var); }
		
		/**
		* Вернуть функцию в виде строки
		*/
		std::string toString() { return func->toString(); }
	};
	
	/**
	* Функция-константа
	*/
	template <class type>
	class MathConst: public MathFunctionImpl<type>
	{
	private:
		type c;
	public:
		MathConst(type v): c(v) { }
		type eval() { return c; }
		MathFunctionImpl<type>* derive(const MathVar<type> &var) { return new MathConst<type>(0); }
		
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
	template <class type>
	class MathVar: public MathFunctionImpl<type>
	{
	private:
		/**
		* Имя переменной
		*/
		const char *name;
		
		/**
		* Значение переменной
		*/
		type value;
	public:
		/**
		* Конструктор переменной
		*/
		MathVar(const char *n): name(n) {}
		
		/**
		* Вернуть название переменной
		*/
		const char *getName() { return name; }
		
		/**
		* Вернуть значение переменной
		*/
		type getValue() { return value; }
		
		/**
		* Установить значение переменной
		*/
		void setValue(const type &v) { value = v; }
		
		/**
		* Вычислить значение переменной
		*/
		type eval() { return value; }
		
		/**
		* Вернуть производную
		*/
		MathFunctionImpl<type>* derive(const MathVar<type> &var) {
			return new MathConst<type>(&var == this ? 1 : 0);
		}
		
		/**
		* Вернуть в виде строки
		*/
		std::string toString() {
			return name;
		}
	};
	
	/**
	* Функция f(x) = -x
	*/
	template <class type>
	class MathNeg: public MathFunctionImpl<type>
	{
	private:
		MathFunction<type> a;
	public:
		MathNeg(MathFunction<type> A): a(A) { }
		type eval() { return - a.eval(); }
		MathFunctionImpl<type>* derive(const MathVar<type> &var) { return new MathNeg(a.derive(var)); }
		
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
	template <class type>
	class MathSum: public MathFunctionImpl<type>
	{
	private:
		MathFunction<type> a;
		MathFunction<type> b;
	public:
		MathSum(const MathFunction<type> A, const MathFunction<type> B): a(A), b(B) { }
		type eval() { return a.eval() + b.eval(); }
		MathFunctionImpl<type>* derive(const MathVar<type> &var) {
			return new MathSum<type>(a.derive(var), b.derive(var));
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
	template <class type>
	class MathMult: public MathFunctionImpl<type>
	{
	private:
		MathFunction<type> a;
		MathFunction<type> b;
	public:
		MathMult(const MathFunction<type> A, const MathFunction<type> B): a(A), b(B) { }
		type eval() { return a.eval() * b.eval(); }
		MathFunctionImpl<type>* derive(const MathVar<type> &var) {
			return new MathSum<type>(
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
	template <class type>
	class MathCos: public MathFunctionImpl<type>
	{
	private:
		MathFunction<type> a;
	public:
		MathCos(const MathFunction<type> A): a(A) { }
		type eval() { return cos(a.eval()); }
		MathFunctionImpl<type>* derive(const MathVar<type> &var);
		
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
	template <class type>
	class MathSin: public MathFunctionImpl<type>
	{
	private:
		MathFunction<type> a;
	public:
		MathSin(const MathFunction<type> A): a(A) { }
		type eval() { return sin(a.eval()); }
		MathFunctionImpl<type>* derive(const MathVar<type> &var);
		
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
	template <class type>
	MathFunctionImpl<type>* MathSin<type>::derive(const MathVar<type> &var)
	{
		return new MathMult<type>(new MathCos<type>(a), a.derive(var));
	}
	
	/**
	* Производная cos(x)
	*/
	template <class type>
	MathFunctionImpl<type>* MathCos<type>::derive(const MathVar<type> &var)
	{
		return new MathNeg<type>(
			new MathMult<type>(new MathSin<type>(a), a.derive(var))
		);
	}
	
	/**
	* Конструктор константной функции
	*/
	template <class type>
	MathFunction<type>::MathFunction(type c)
	{
		func = new MathConst<type>(c);
		func->lock();
	}
	
	/**
	* Сумма функций
	*/
	template <class type> MathFunction<type> operator + (MathFunction<type> a, MathFunction<type> b)
	{
		return new MathSum<type>(a, b);
	}
	
	/**
	* Сумма функции и константы
	*/
	template <class type> MathFunction<type> operator + (MathFunction<type> a, type b)
	{
		return new MathSum<type>(a, MathFunction<type>(b));
	}
	
	/**
	* Сумма функции и константы
	*/
	template <class type> MathFunction<type> operator + (type a, MathFunction<type> b)
	{
		return new MathSum<type>(MathFunction<type>(a), b);
	}
	
	/**
	* Произведение функций
	*/
	template <class type> MathFunction<type> operator * (MathFunction<type> a, MathFunction<type> b)
	{
		return new MathMult<type>(a, b);
	}
	
	/**
	* Произведение функции и константы
	*/
	template <class type> MathFunction<type> operator * (MathFunction<type> a, type b)
	{
		return new MathMult<type>(a, MathFunction<type>(b));
	}
	
	/**
	* Произведение функции и константы
	*/
	template <class type> MathFunction<type> operator * (type a, MathFunction<type> b)
	{
		return new MathMult<type>(MathFunction<type>(a), b);
	}
	
	/**
	* Функция sin(x)
	*/
	template <class type> MathFunction<type> sin(MathFunction<type> x)
	{
		return new MathSin<type>(x);
	}
	
	/**
	* Функция cos(x)
	*/
	template <class type> MathFunction<type> cos(MathFunction<type> x)
	{
		return new MathCos<type>(x);
	}
}

#endif // NANOSOFT_MATH_PARSER_H
