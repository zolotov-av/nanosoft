#ifndef NANOSOFT_MATH_H
#define NANOSOFT_MATH_H

#include <math.h>
#include <string>
#include <stdio.h>

namespace nanosoft
{
	/**
	* Абстрактный класс реализации метематической функции
	*
	* Внутренняя структура для объявления пользовательских функций.
	*
	* Для непосредственных вычислений используйте классы
	* MathFunction, MathVar и функции sin(), cos() и т.п.
	*/
	class MathFunctionImpl
	{
	private:
		/**
		* Счетчик ссылок
		*/
		int ref_count;
		std::string id;
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
		* Вернуть тип функции
		*/
		virtual std::string getType() = 0;
		
		/**
		* Вычисление значения функции
		*/
		virtual double eval() = 0;
		
		/**
		* Вернуть производную функции
		* @param var переменная по которой производиться дифференцирование
		*/
		virtual class MathFunction derive(const class MathVar &var) = 0;
		
		/**
		* Вернуть оптимизированную функцию
		*/
		virtual class MathFunction optimize();
		
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
	* Класс-контейнер для переменной математичекой функции
	*/
	class MathVar
	{
	private:
		class MathVarImpl *var;
	public:
		MathVar();
		MathVar(const char *name, double value = 0.0);
		MathVar(const MathVar &v);
		~MathVar();
		
		/**
		* Вернуть название переменной
		*/
		const char *getName() const;
		
		/**
		* Вернуть значение переменной
		*/
		double getValue() const;
		
		/**
		* Установить значение переменной
		*/
		void setValue(double v);
		
		/**
		* Оператор копирования переменной
		*/
		void operator = (const MathVar &a);
		
		/**
		* Сравнение переменных
		* Внимание, сравнивается не значение, а "имя" переменной
		*/
		bool operator == (const MathVar &v) const { return var == v.var; }
		
		/**
		* Сравнение переменных
		* Внимание, сравнивается не значение, а "имя" переменной
		*/
		bool operator == (const MathVarImpl *v) const { return var == v; }
		
		/**
		* Сравнение переменных
		* Внимание, сравнивается не значение, а "имя" переменной
		*/
		bool operator != (const MathVar &v) const { return var != v.var; }
		
		/**
		* Вернуть функцию f(x) = x
		*/
		operator class MathFunction ();
	};
	
	/**
	* Класс-контейнер для математичекой функции
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
		MathFunction(MathFunctionImpl *impl);
		
		/**
		* Конструктор копии
		*/
		MathFunction(const MathFunction &f);
		
		/**
		* Деструктор функции
		*/
		~MathFunction();
		
		/**
		* Вернуть тип функции
		*/
		std::string getType() { return func->getType(); }
		
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
		* Вернуть оптимизированную функцию
		*/
		MathFunction optimize() { return func->optimize(); }
		
		void operator = (const MathFunction &a)
		{
			if ( func ) func->release();
			func = a.func;
			func->lock();
		}
		
		/**
		* Вернуть функцию в виде строки
		*/
		std::string toString() { return func->toString(); }
		
		template <class type>
		type* cast() const { return dynamic_cast<type*>(func); }
	};
	
	/**
	* Унарный оператор вычитания
	*/
	MathFunction operator - (const MathFunction &a);
	
	/**
	* Сумма функций
	*/
	MathFunction operator + (const MathFunction &a, const MathFunction &b);
	
	/**
	* Сумма функции и константы
	*/
	MathFunction operator + (const MathFunction &a, double b);
	
	/**
	* Сумма функции и константы
	*/
	MathFunction operator + (double a, const MathFunction &b);
	
	/**
	* Разность функций
	*/
	MathFunction operator - (const MathFunction &a, const MathFunction &b);
	
	/**
	* Сумма функции и константы
	*/
	MathFunction operator - (const MathFunction &a, double b);
	
	/**
	* Сумма функции и константы
	*/
	MathFunction operator - (double a, const MathFunction &b);
	
	/**
	* Произведение функций
	*/
	MathFunction operator * (const MathFunction &a, const MathFunction &b);
	
	/**
	* Произведение функции и константы
	*/
	MathFunction operator * (const MathFunction &a, double b);
	
	/**
	* Произведение функции и константы
	*/
	MathFunction operator * (double a, const MathFunction &b);
	
	/**
	* Деление функций
	*/
	MathFunction operator / (const MathFunction &a, const MathFunction &b);
	
	/**
	* Деление функции на константу
	*/
	MathFunction operator / (const MathFunction &a, double b);
	
	/**
	* Деление константы на функцию
	*/
	MathFunction operator / (double a, const MathFunction &b);
	
	/**
	* Функция sin(x)
	*/
	MathFunction sin(const MathFunction &x);
	
	/**
	* Функция cos(x)
	*/
	MathFunction cos(const MathFunction &x);
	
	/**
	* Функция exp(x)
	*/
	MathFunction exp(const MathFunction &x);
	
	/**
	* Функция ln(x) (натуальный логарифм)
	*/
	MathFunction ln(const MathFunction &x);
	
	/**
	* Функция pow(x, y) = x ^ y
	*/
	MathFunction pow(const MathFunction &x, const MathFunction &y);
}

#endif // NANOSOFT_MATH_H
