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
		* Первый этап оптимизации (константы, суммы, произведения)
		*/
		virtual class MathFunction optimize1() = 0;
		
		/**
		* Второй этап оптимизации (светывание разностей и частных)
		*/
		virtual class MathFunction optimize2() = 0;
		
		/**
		* Вернуть в виде строки
		*/
		virtual std::string toString() = 0;
		
		/**
		* Вернуть в виде строки для отладки и тестирования
		*/
		virtual std::string debugString() = 0;
		
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
		operator class MathFunction () const;
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
		* Конструктор по умолчанию (нуль-функция)
		*/
		MathFunction();
		
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
		std::string getType() const;
		
		/**
		* Вычислить функцию
		*/
		double eval() const;
		
		/**
		* Вернуть производную функции
		* @param var переменная по которой будет дифференцирование
		*/
		MathFunction derive(const MathVar &var) const;
		
		/**
		* Вернуть оптимизированную функцию
		*/
		MathFunction optimize1() const;
		
		/**
		* Вернуть оптимизированную функцию
		*/
		MathFunction optimize2() const;
		
		/**
		* Вернуть оптимизированную функцию
		*/
		MathFunction optimize() const;
		
		void operator = (const MathFunction &a);
		
		/**
		* Вернуть функцию в виде строки
		*/
		std::string toString() const;
		
		/**
		* Вернуть в виде строки для отладки и тестирования
		*/
		std::string debugString() const;
		
		template <class type>
		type* cast() const { return dynamic_cast<type*>(func); }
	};
	
	/**
	* Тип конструктора функции одной переменной
	*/
	typedef MathFunction (*MathFunctionX)(const MathFunction &x);
	
	/**
	* Типа конструктора функции двух переменных
	*/
	typedef MathFunction (*MathFunctionXY)(const MathFunction &x, const MathFunction &y);
	
	/**
	* Первый этап оптимизатора по умолчанию для функции одной переменной
	*/
	MathFunction opt1_default(MathFunctionX f, const MathFunction &a);
	
	/**
	* Второй этап оптимизатора по умолчанию для функции одной переменной
	*/
	MathFunction opt2_default(MathFunctionX f, const MathFunction &a);
	
	/**
	* Первый этап оптимизатора по умолчанию для функции двух переменных
	*/
	MathFunction opt1_default(MathFunctionXY f, const MathFunction &a, const MathFunction &b);
	
	/**
	* Втрой этап оптимизатора по умолчанию для функции двух переменных
	*/
	MathFunction opt2_default(MathFunctionXY f, const MathFunction &a, const MathFunction &b);
	
	/**
	* Оптимизатор четной функции
	*/
	MathFunction opt1_even(MathFunctionX f, const MathFunction &a);
	
	/**
	* Оптимизатор нечетной функции
	*/
	MathFunction opt1_odd(MathFunctionX f, const MathFunction &a);
	
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
	* Функция inv(x) = 1 / x
	*/
	MathFunction inv(const MathFunction &a);
	
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
	* Функция pow(x, y) = x ^ y
	*/
	MathFunction pow(const MathFunction &x, const MathFunction &y);
	
	/**
	* Функция ln(x) (натуальный логарифм)
	*/
	MathFunction ln(const MathFunction &x);
	
	/**
	* Функция log(x, a) - логарифм числа x по основанию a
	*/
	MathFunction log(const MathFunction &x, const MathFunction &a);
	
	/**
	* Оптимизация функции
	*/
	MathFunction optimize(const MathFunction &f);
	
	/**
	* Производная функции
	*/
	MathFunction derive(const MathFunction &f, const MathVar &var);
}

#endif // NANOSOFT_MATH_H
