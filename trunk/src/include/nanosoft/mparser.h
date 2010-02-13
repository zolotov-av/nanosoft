#ifndef NANOSOFT_MATH_PARSER_H
#define NANOSOFT_MATH_PARSER_H

#include <math.h>
#include <string>
#include <stdio.h>

namespace nanosoft
{
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
		virtual type eval(type x) = 0;
		
		/**
		* Вернуть производную функции
		*/
		virtual MathFunctionImpl<type>* derive() = 0;
		
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
		type eval(type x) { return func->eval(x); }
		
		/**
		* Вернуть производную функции
		*/
		MathFunction<type> derive() { return func->derive(); }
		
		/**
		* Вернуть функцию в виде строки
		*/
		std::string toString() { return func->toString(); }
		
		/**
		* Вычислить функцию
		*/
		type operator () (type x) { return func->eval(x); }
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
		type eval(type x) { return c; }
		MathFunctionImpl<type>* derive() { return new MathConst<type>(0); }
		
		/**
		* Вернуть в виде строки
		*/
		std::string toString() {
			char buf[80];
			int len = sprintf(buf, "%f", c);
			return std::string(buf, len);
		}
	};
	
	template <class type>
	class MathNeg: public MathFunctionImpl<type>
	{
	private:
		MathFunction<type> a;
	public:
		MathNeg(MathFunction<type> A): a(A) { }
		type eval(type x) { return - a.eval(x); }
		MathFunctionImpl<type>* derive() { return new MathNeg(a.derive()); }
		
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
		type eval(type x) { return a.eval(x) + b.eval(x); }
		MathFunctionImpl<type>* derive() { return new MathSum<type>(a.derive(), b.derive()); }
		
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
		type eval(type x) { return a.eval(x) * b.eval(x); }
		MathFunctionImpl<type>* derive() {
			return new MathSum<type>(
				new MathMult(a.derive(), b),
				new MathMult(a, b.derive())
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
	public:
		type eval(type x) { return cos(x); }
		MathFunctionImpl<type>* derive();
		
		/**
		* Вернуть в виде строки
		*/
		std::string toString() {
			return "cos(x)";
		}
	};
	
	/**
	* Функция F(x) = sin(x)
	*/
	template <class type>
	class MathSin: public MathFunctionImpl<type>
	{
	public:
		type eval(type x) { return sin(x); }
		MathFunctionImpl<type>* derive();
		
		/**
		* Вернуть в виде строки
		*/
		std::string toString() {
			return "sin(x)";
		}
	};
	
	template <class type>
	MathFunctionImpl<type>* MathSin<type>::derive()
	{
		return new MathCos<type>();
	}
	
	template <class type>
	MathFunctionImpl<type>* MathCos<type>::derive()
	{
		return new MathNeg<type>(new MathSin<type>());
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
	template <class type> MathFunction<type> sin()
	{
		return new MathSin<type>();
	}
	
	/**
	* Функция cos(x)
	*/
	template <class type> MathFunction<type> cos()
	{
		return new MathCos<type>();
	}
}

#endif // NANOSOFT_MATH_PARSER_H
