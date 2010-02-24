#ifndef NANOSOFT_MATHPARSER_H
#define NANOSOFT_MATHPARSER_H

#include <nanosoft/math.h>

#include <string>
#include <exception>

#include <stdio.h>
#include <map>

namespace nanosoft
{
	/**
	* Тип конструктора функции одной переменной
	*/
	typedef MathFunction (*MathFunctionX)(const MathFunction &x);
	
	/**
	* Типа конструктора функции двух переменных
	*/
	typedef MathFunction (*MathFunctionXY)(const MathFunction &x, const MathFunction &y);
	
	/**
	* Класс ошибки парсера математических функций
	*/
	class MathParserError: public std::exception
	{
	private:
		/**
		* Сообщение об ошибке
		*/
		std::string message;
	public:
		/**
		* Конструктор ошибки
		*/
		MathParserError(const std::string &Message);
		
		/**
		* Деструктор ошибки
		*/
		virtual ~MathParserError() throw ();
		
		/**
		* Вернуть сообщение об ошибке
		*/
		virtual const char *what() const throw ();
	};
	
	/**
	* Парсер математических функций
	*/
	class MathParser
	{
	public:
		/**
		* Типы лексем
		*/
		enum token_type {
			tok_end, // конец выражения
			tok_error, // признак ситаксической ошибки
			tok_number, // число
			tok_name, // имя переменной или функции
			tok_plus, tok_minus, tok_mult, tok_div, tok_pow, // операторы
			tok_open, tok_close, // скобки
			tok_comma // запятая (разделить аргументов функции)
		};
		
		/**
		* Описание лексемы
		*/
		struct Token
		{
			/**
			* Типа лексемы
			*/
			token_type type;
			
			/**
			* Указатель на начало лексемы
			*/
			const char *begin;
			
			/**
			* Указатель на конец лексемы
			*/
			const char *limit;
			
			/**
			* Значение лексемы
			*/
			std::string value;
		};
		
		typedef std::map<std::string, class MathHelperParser *> names_t;
		names_t names;
		
		/**
		* Парсинг лексемы
		* @param token стуктура принимающая описание лексемы
		* @param expr выражение
		* @param limit ограничение строки
		* @return тип лексемы
		*/
		token_type parseToken(Token &token, const char *&expr, const char *limit);
		
		/**
		* Парсинг подвыражения в скобках
		* Если выражение содержит синтаксическую ошибку, то
		* генерируется исключение
		* @param expr выражение для парсинга
		* @param limit ограничение
		* @return математическая фунция
		*/
		MathFunction parseSubExpr(const char *&expr, const char *limit);
		
		/**
		* Парсинг переменной и вызова функций
		* Если выражение содержит синтаксическую ошибку, то
		* генерируется исключение
		* @param expr выражение для парсинга
		* @param limit ограничение
		* @return математическая фунция
		*/
		MathFunction parseNamedObject(const Token &token, const char *&expr, const char *limit);
		
		/**
		* Парсинг атомарного выражения
		* Если выражение содержит синтаксическую ошибку, то
		* генерируется исключение
		* @param expr выражение для парсинга
		* @param limit ограничение
		* @return математическая фунция
		*/
		MathFunction parseAtom(const char *&expr, const char *limit);
		
		/**
		* Парсинг произведения
		* Если выражение содержит синтаксическую ошибку, то
		* генерируется исключение
		* @param expr выражение для парсинга
		* @param limit ограничение
		* @return математическая фунция
		*/
		MathFunction parseMult(const char *&expr, const char *limit);
		
		/**
		* Парсинг суммы
		* Если выражение содержит синтаксическую ошибку, то
		* генерируется исключение
		* @param expr выражение для парсинга
		* @param limit ограничение
		* @return математическая фунция
		*/
		MathFunction parseSum(const char *&expr, const char *limit);
	public:
		/**
		* Конструктор парсера
		*/
		MathParser();
		
		/**
		* Деструктор парсера
		*/
		~MathParser();
		
		/**
		* Добавить переменную
		*/
		void set(const char *name, const MathVar &var);
		
		/**
		* Добавить функцию одной переменной
		*/
		void set(const char *name, const MathFunctionX f);
		
		/**
		* Добавить функцию двух переменных
		*/
		void set(const char *name, const MathFunctionXY f);
		
		/**
		* Проверить зарегистрированное ли имя
		*/
		bool isset(const char *name);
		
		/**
		* Удалить переменную/функцию
		*/
		void unset(const char *name);
		
		/**
		* Парсинг выражения
		* Если выражение содержит синтаксическую ошибку, то
		* генерируется исключение
		* @param expr выражение для парсинга
		* @return математическая фунция
		*/
		MathFunction parse(const char *expr);
		
		/**
		* Парсинг выражения
		* Если выражение содержит синтаксическую ошибку, то
		* генерируется исключение
		* @param expr выражение для парсинга
		* @param len длина выражения
		* @return математическая фунция
		*/
		MathFunction parse(const char *expr, size_t len);
		
		/**
		* Парсинг выражения
		* Если выражение содержит синтаксическую ошибку, то
		* генерируется исключение
		* @param expr выражение для парсинга
		* @return математическая фунция
		*/
		MathFunction parse(const std::string &expr);
	};
}

#endif // NANOSOFT_MATHPARSER_H
