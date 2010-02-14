#ifndef NANOSOFT_MATH_H
#define NANOSOFT_MATH_H

#include <math.h>
#include <string>
#include <stdio.h>

namespace nanosoft
{
	class MathVar;
	
	/**
	* Абстрактный класс реализации метематической функции
	*/
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
		MathFunctionImpl();
		
		/**
		* Виртуальный деструктор
		*/
		virtual ~MathFunctionImpl();
		
		/**
		* Вычисление значения функции
		*/
		virtual double eval() = 0;
		
		/**
		* Вернуть производную функции
		* @param var переменная по которой производиться дифференцирование
		*/
		virtual MathFunctionImpl* derive(const MathVar &var) = 0;
		
		/**
		* Вернуть в виде строки
		*/
		virtual std::string toString() = 0;
		
		/**
		* Увеличить счетчик ссылок
		*/
		void lock();
		
		/**
		* Уменьшить счетчик ссылок, если станет 0, то самоудалиться
		*/
		void release();
	};
	
	/**
	* Класс математичекой функции
	*/
	class MathFunction
	{
	private:
		/**
		* Реализация функции
		*/
		MathFunctionImpl *func;
	public:
		/**
		* Конструктор константной функции
		*/
		MathFunction(double c);
		
		/**
		* Конструктор функции
		*/
		MathFunction(MathFunctionImpl *impl) { func = impl; }
		
		/**
		* Вычислить функцию
		*/
		double eval() { return func->eval(); }
		
		/**
		* Вернуть производную функции
		* @param var переменная по которой будет дифференцирование
		*/
		MathFunction derive(const MathVar &var) { return func->derive(var); }
		
		/**
		* Вернуть функцию в виде строки
		*/
		std::string toString() { return func->toString(); }
	};
	
	/**
	* Класс переменной
	*/
	class MathVar: public MathFunctionImpl
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
		MathVar(const char *n);
		
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
		MathFunctionImpl* derive(const MathVar &var);
		
		/**
		* Вернуть в виде строки
		*/
		std::string toString();
	};
	
	/**
	* Сумма функций
	*/
	MathFunction operator + (MathFunction a, MathFunction b);
	
	/**
	* Сумма функции и константы
	*/
	MathFunction operator + (MathFunction a, double b);
	
	/**
	* Сумма функции и константы
	*/
	MathFunction operator + (double a, MathFunction b);
	
	/**
	* Произведение функций
	*/
	MathFunction operator * (MathFunction a, MathFunction b);
	
	/**
	* Произведение функции и константы
	*/
	MathFunction operator * (MathFunction a, double b);
	
	/**
	* Произведение функции и константы
	*/
	MathFunction operator * (double a, MathFunction b);
	
	/**
	* Функция sin(x)
	*/
	MathFunction sin(MathFunction x);
	
	/**
	* Функция cos(x)
	*/
	MathFunction cos(MathFunction x);
}

#endif // NANOSOFT_MATH_H
