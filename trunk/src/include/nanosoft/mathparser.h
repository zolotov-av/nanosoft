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
	friend class MathHelperParser;
	friend class MathVarParser;
	friend class MathFunctionXParser;
	friend class MathFunctionXYParser;
	private:
		/**
		* Типы лексем
		*/
		enum token_type {
			tok_end, // конец выражения
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
			* Вернуть тип лексемы в виде строки
			*/
			std::string typeString() const;
			
			/**
			* Вернуть лексему в виде строки
			*/
			std::string toString() const { return std::string(begin, limit); }
		};
		
		typedef std::map<std::string, class MathHelperParser *> names_t;
		names_t names;
		
		/**
		* Парсинг лексемы числа
		* @param token стуктура принимающая описание лексемы
		* @param expr выражение
		* @param limit ограничение строки
		* @return тип лексемы
		*/
		static token_type parseNumber(Token &token, const char *&expr, const char *limit);
		
		/**
		* Парсинг лексемы идентификатора (переменной, функции)
		* @param token стуктура принимающая описание лексемы
		* @param expr выражение
		* @param limit ограничение строки
		* @return тип лексемы
		*/
		static token_type parseName(Token &token, const char *&expr, const char *limit);
		
		/**
		* Парсинг лексемы
		* @param token стуктура принимающая описание лексемы
		* @param expr выражение
		* @param limit ограничение строки
		* @return тип лексемы
		*/
		static token_type parseToken(Token &token, const char *&expr, const char *limit);
		
		/**
		* Парсинг переменной и вызова функций
		* Если выражение содержит синтаксическую ошибку, то
		* генерируется исключение
		* @param name имя объекта
		* @param expr выражение для парсинга
		* @param limit ограничение
		* @return математическая фунция
		*/
		MathFunction parseNamedObject(const std::string &name, const char *&expr, const char *limit) const;
		
		/**
		* Парсинг подвыражения в скобках
		* Если выражение содержит синтаксическую ошибку, то
		* генерируется исключение
		* @param expr выражение для парсинга
		* @param limit ограничение
		* @return математическая фунция
		*/
		MathFunction parseSubExpr(const char *&expr, const char *limit) const;
		
		/**
		* Парсинг атомарного выражения
		* Если выражение содержит синтаксическую ошибку, то
		* генерируется исключение
		* @param expr выражение для парсинга
		* @param limit ограничение
		* @return математическая фунция
		*/
		MathFunction parseAtom(const char *&expr, const char *limit) const;
		
		/**
		* Парсинг произведения
		* Если выражение содержит синтаксическую ошибку, то
		* генерируется исключение
		* @param expr выражение для парсинга
		* @param limit ограничение
		* @return математическая фунция
		*/
		MathFunction parseMult(const char *&expr, const char *limit) const;
		
		/**
		* Парсинг суммы
		* Если выражение содержит синтаксическую ошибку, то
		* генерируется исключение
		* @param expr выражение для парсинга
		* @param limit ограничение
		* @return математическая фунция
		*/
		MathFunction parseSum(const char *&expr, const char *limit) const;
	protected:
		/**
		* Подключение функций
		*
		* Вызывается в констукторе для автоматического подключения
		* функций. Чтобы добавить, удалить или переопределить
		* какие-то функции можно переопределить эту функцию
		*/
		virtual void bindFunctions();
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
		* Добавить параметр/переменную
		*/
		void setVar(const char *name, const MathVar &value);
		
		/**
		* Добавить параметр/переменную
		*/
		void setVar(const char *name, const MathFunction &value);
		
		/**
		* Добавить константу
		*/
		void setConst(const char *name, double value);
		
		/**
		* Добавить функцию одной переменной
		*/
		void setFunction(const char *name, const MathFunctionX f);
		
		/**
		* Добавить функцию двух переменных
		*/
		void setFunction(const char *name, const MathFunctionXY f);
		
		/**
		* Проверить зарегистрированное ли имя
		*/
		bool isset(const char *name) const;
		
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
		MathFunction parse(const char *expr) const;
		
		/**
		* Парсинг выражения
		* Если выражение содержит синтаксическую ошибку, то
		* генерируется исключение
		* @param expr выражение для парсинга
		* @param len длина выражения
		* @return математическая фунция
		*/
		MathFunction parse(const char *expr, size_t len) const;
		
		/**
		* Парсинг выражения
		* Если выражение содержит синтаксическую ошибку, то
		* генерируется исключение
		* @param expr выражение для парсинга
		* @return математическая фунция
		*/
		MathFunction parse(const std::string &expr) const;
	};
}

#endif // NANOSOFT_MATHPARSER_H
