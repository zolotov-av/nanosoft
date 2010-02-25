#include <nanosoft/mathparser.h>

#include <string>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

namespace nanosoft
{
	/**
	* Конструктор ошибки
	*/
	MathParserError::MathParserError(const std::string &Message): message(Message)
	{
	}
	
	/**
	* Деструктор ошибки
	*/
	MathParserError::~MathParserError() throw ()
	{
	}
	
	/**
	* Вернуть сообщение об ошибке
	*/
	const char * MathParserError::what() const throw ()
	{
		return message.c_str();
	}
	
	/**
	* Вспомогательный класс парсера именованного объекта (переменных и функций)
	*/
	class MathHelperParser
	{
	public:
		/**
		* Конструктор
		*/
		MathHelperParser() { }
		
		/**
		* Деструктор
		*/
		virtual ~MathHelperParser() { }
		
		/**
		* Парсер подвыражения
		*/
		virtual MathFunction parse(const MathParser *p, const char *&expr, const char *limit) = 0;
	};
	
	/**
	* Класс парсера переменных
	*/
	class MathVarParser: public MathHelperParser
	{
	private:
		/**
		* Переменная
		* @note ага, может быть и функцией и константой
		*/
		MathFunction var;
	public:
		/**
		* Конструктор парсера переменной
		*/
		MathVarParser(const MathFunction &f): var(f) { }
		
		/**
		* Парсер переменной
		*/
		MathFunction parse(const MathParser *p, const char *&expr, const char *limit) {
			return var;
		}
	};
	
	/**
	* Класс парсера функций одной переменной
	*/
	class MathFunctionXParser: public MathHelperParser
	{
	private:
		/**
		* Конструктор функции одной переменной
		*/
		MathFunctionX func;
	public:
		/**
		* Конструктор парсера функции одной переменной
		*/
		MathFunctionXParser(const MathFunctionX &f): func(f) { }
		
		/**
		* Парсер функции одной переменной
		*/
		MathFunction parse(const MathParser *p, const char *&expr, const char *limit)
		{
			MathParser::Token token;
			
			if ( p->parseToken(token, expr, limit) != MathParser::tok_open )
			{
				throw MathParserError("expected subexpression");
			}
			
			MathFunction x = p->parseSum(expr, limit);
			
			if ( p->parseToken(token, expr, limit) != MathParser::tok_close )
			{
				throw MathParserError("expected end of expression");
			}
			
			return func(x);
		}
	};
	
	/**
	* Класс парсера функции двух переменных
	*/
	class MathFunctionXYParser: public MathHelperParser
	{
	private:
		/**
		* Конструктор функци двух переменных
		*/
		MathFunctionXY func;
	public:
		/**
		* Конструктор парсера функции двух переменных
		*/
		MathFunctionXYParser(const MathFunctionXY &f): func(f) { }
		
		/**
		* Парсер функции двух переменных
		*/
		MathFunction parse(const MathParser *p, const char *&expr, const char *limit)
		{
			MathParser::Token token;
			
			if ( p->parseToken(token, expr, limit) != MathParser::tok_open )
			{
				throw MathParserError("expected subexpression");
			}
			
			MathFunction x = p->parseSum(expr, limit);
			
			if ( p->parseToken(token, expr, limit) != MathParser::tok_comma )
			{
				throw MathParserError("expected second parameter in function");
			}
			
			MathFunction y = p->parseSum(expr, limit);
			
			if ( p->parseToken(token, expr, limit) != MathParser::tok_close )
			{
				throw MathParserError("expected end of expression");
			}
			
			return func(x, y);
		}
	};
	
	/**
	* Вернуть тип лексемы в виде строки
	*/
	std::string MathParser::Token::typeString() const
	{
		static const char *buf[tok_comma+1] = {
		"end",
		"number",
		"name",
		"operator +",
		"operator -",
		"operator *",
		"operator /",
		"operator ^",
		"open",
		"close",
		"comma"
		};
		return buf[type];
	}
	
	/**
	* Конструктор парсера
	*/
	MathParser::MathParser()
	{
		bindFunctions();
	}
	
	/**
	* Деструктор парсера
	*/
	MathParser::~MathParser()
	{
	}
	
	/**
	* Добавить переменную
	*/
	void MathParser::setVar(const char *name, const MathVar &value)
	{
		names_t::iterator iter = names.find(name);
		if ( iter != names.end() ) delete iter->second;
		names[name] = new MathVarParser(value);
	}
	
	/**
	* Добавить параметр/переменную
	*/
	void MathParser::setVar(const char *name, const MathFunction &value)
	{
		names_t::iterator iter = names.find(name);
		if ( iter != names.end() ) delete iter->second;
		names[name] = new MathVarParser(value);
	}
	
	/**
	* Добавить константу
	*/
	void MathParser::setConst(const char *name, double value)
	{
		names_t::iterator iter = names.find(name);
		if ( iter != names.end() ) delete iter->second;
		names[name] = new MathVarParser(value);
	}
	
	/**
	* Добавить функцию одной переменной
	*/
	void MathParser::setFunction(const char *name, const MathFunctionX f)
	{
		names_t::iterator iter = names.find(name);
		if ( iter != names.end() ) delete iter->second;
		names[name] = new MathFunctionXParser(f);
	}
	
	/**
	* Добавить функцию двух переменных
	*/
	void MathParser::setFunction(const char *name, const MathFunctionXY f)
	{
		names_t::iterator iter = names.find(name);
		if ( iter != names.end() ) delete iter->second;
		names[name] = new MathFunctionXYParser(f);
	}
	
	/**
	* Проверить зарегистрированное ли имя
	*/
	bool MathParser::isset(const char *name) const
	{
		names_t::const_iterator iter = names.find(name);
		return iter != names.end();
	}
	
	/**
	* Удалить переменную/функцию
	*/
	void MathParser::unset(const char *name)
	{
		names_t::iterator iter = names.find(name);
		if ( iter != names.end() ) delete iter->second;
		names.erase(iter);
	}
	
	/**
	* Подключение функций
	*
	* Вызывается в констукторе для автоматического подключения
	* функций. Чтобы добавить, удалить или переопределить
	* какие-то функции можно переопределить эту функцию
	*/
	void MathParser::bindFunctions()
	{
		// базовые константы
		setConst("pi", M_PI);
		setConst("e", M_E);
		
		// арифметрика
		setFunction("exp", nanosoft::exp);
		setFunction("pow", nanosoft::pow);
		setFunction("ln", nanosoft::ln);
		setFunction("log", nanosoft::log);
		
		// тригонометрия
		setFunction("sin", nanosoft::sin);
		setFunction("cos", nanosoft::cos);
		
		// дополнительные вкусности
		setFunction("optimize", nanosoft::optimize);
	}
	
	/**
	* Парсинг лексемы числа
	* @param token стуктура принимающая описание лексемы
	* @param expr выражение
	* @param limit ограничение строки
	* @return тип лексемы
	*/
	MathParser::token_type MathParser::parseNumber(MathParser::Token &token, const char *&expr, const char *limit)
	{
		const char *p = expr + 1;
		while ( *p >= '0' && *p <= '9' ) p++;
		if ( *p == '.' )
		{
			p++;
			while ( *p >= '0' && *p <= '9' ) p++;
		}
		int len = p - expr;
		token.type = MathParser::tok_number;
		token.begin = expr;
		token.limit = p;
		expr = p;
		return token.type;
	}
	
	/**
	* Парсинг лексемы идентификатора (переменной, функции)
	* @param token стуктура принимающая описание лексемы
	* @param expr выражение
	* @param limit ограничение строки
	* @return тип лексемы
	*/
	MathParser::token_type MathParser::parseName(MathParser::Token &token, const char *&expr, const char *limit)
	{
		const char *p = expr + 1;
		while ( *p >= 'A' && *p <= 'Z'
			|| *p >= 'a' && *p <= 'z'
			|| *p >= '0' && *p <= '9'
			|| *p == '_' ) p++;
		int len = p - expr;
		token.type = MathParser::tok_name;
		token.begin = expr;
		token.limit = p;
		expr = p;
		return token.type;
	}
	
	/**
	* Парсинг лексемы
	* @param token стуктура принимающая описание лексемы
	* @param expr выражение
	* @param limit ограничение строки
	* @return тип лексемы
	*/
	MathParser::token_type MathParser::parseToken(MathParser::Token &token, const char *&expr, const char *limit)
	{
		// пропустить пробелы
		const char *p = expr;
		while ( *p == ' ' || *p == '\n' || *p == '\r' || *p == '\t' ) p++;
		expr = p;
		char c = *p;
		token.begin = p;
		token.limit = p + 1;
		switch ( c )
		{
		case '\0': return token.type = tok_end;
		case '+': { expr++; return token.type = tok_plus; }
		case '-': { expr++; return token.type = tok_minus; }
		case '*': { expr++; return token.type = tok_mult; }
		case '/': { expr++; return token.type = tok_div; }
		case '^': { expr++; return token.type = tok_pow; }
		case '(': { expr++; return token.type = tok_open; }
		case ')': { expr++; return token.type = tok_close; }
		case ',': { expr++; return token.type = tok_comma; }
		}
		if ( c >= '0' && c <= '9' ) return parseNumber(token, expr, limit);
		if ( c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z' || c == '_' ) return parseName(token, expr, limit);
		throw MathParserError("unexpected character: '" + std::string(&c, 1) + "'");
	}
	
	/**
	* Парсинг переменной и вызова функций
	* Если выражение содержит синтаксическую ошибку, то
	* генерируется исключение
	* @param name имя объекта
	* @param expr выражение для парсинга
	* @param limit ограничение
	* @return математическая фунция
	*/
	MathFunction MathParser::parseNamedObject(const std::string &name, const char *&expr, const char *limit) const
	{
		names_t::const_iterator iter = names.find(name);
		if ( iter != names.end() ) {
			return iter->second->parse(this, expr, limit);
		}
		throw MathParserError("unknown variable or function: " + name);
	}
	
	/**
	* Парсинг подвыражения в скобках
	* Если выражение содержит синтаксическую ошибку, то
	* генерируется исключение
	* @param expr выражение для парсинга
	* @param limit ограничение
	* @return математическая фунция
	*/
	MathFunction MathParser::parseSubExpr(const char *&expr, const char *limit) const
	{
		Token token;
		
		MathFunction f = parseSum(expr, limit);
		
		if ( parseToken(token, expr, limit) != tok_close )
		{
			throw MathParserError("expected end of expression");
		}
		
		return f;
	}
	
	/**
	* Парсинг атомарного выражения
	* Если выражение содержит синтаксическую ошибку, то
	* генерируется исключение
	* @param expr выражение для парсинга
	* @param limit ограничение
	* @return математическая фунция
	*/
	MathFunction MathParser::parseAtom(const char *&expr, const char *limit) const
	{
		Token token;
		switch ( parseToken(token, expr, limit) )
		{
		case tok_end: throw MathParserError("unexpected end of expression");
		case tok_number: return atof(token.toString().c_str());
		case tok_name: return parseNamedObject(token.toString(), expr, limit);
		case tok_open: return parseSubExpr(expr, limit);
		default: throw MathParserError("unexpected");
		}
	}
	
	/**
	* Парсинг произведения
	* Если выражение содержит синтаксическую ошибку, то
	* генерируется исключение
	* @param expr выражение для парсинга
	* @param limit ограничение
	* @return математическая фунция
	*/
	MathFunction MathParser::parseMult(const char *&expr, const char *limit) const
	{
		Token token;
		token_type op;
		
		// парсим первый множитель
		MathFunction mult = parseAtom(expr, limit);
		
		while ( 1 )
		{
			// парсим оператор
			switch ( parseToken(token, expr, limit) )
			{
			case tok_mult:
			case tok_div:
				op = token.type;
				break;
			default:
				expr = token.begin;
				return mult;
			}
			
			// парсим очередной множитель
			if ( op == tok_mult ) mult = mult * parseMult(expr, limit);
			else mult = mult / parseMult(expr, limit);
		}
		
		return mult;
	}
	
	/**
	* Парсинг суммы
	* Если выражение содержит синтаксическую ошибку, то
	* генерируется исключение
	* @param expr выражение для парсинга
	* @param limit ограничение
	* @return математическая фунция
	*/
	MathFunction MathParser::parseSum(const char *&expr, const char *limit) const
	{
		Token token;
		
		// проверка на унарный минус/плюс
		token_type op = parseToken(token, expr, limit);
		if ( op != tok_minus && op != tok_plus ) expr = token.begin;
		
		// парсим первое слагаемое
		MathFunction sum = parseMult(expr, limit);
		
		// если вначале выражения был унарный минус,
		// то инвертировать певрое слагаемое
		if ( op == tok_minus ) sum = - sum;
		
		while ( 1 )
		{
			// парсим оператор
			switch ( parseToken(token, expr, limit) )
			{
			case tok_plus:
			case tok_minus:
				op = token.type;
				break;
			default:
				expr = token.begin;
				return sum;
			}
			
			// парсим очередное слагаемое
			if ( op == tok_plus ) sum = sum + parseMult(expr, limit);
			else sum = sum - parseMult(expr, limit);
		}
		
		return sum;
	}
	
	/**
	* Парсинг выражения
	* Если выражение содержит синтаксическую ошибку, то
	* генерируется исключение
	* @param expr выражение для парсинга
	* @param len длина выражения
	* @return математическая фунция
	*/
	MathFunction MathParser::parse(const char *expr, size_t len) const
	{
		const char *limit = expr + len;
		
		// парсим сумму
		MathFunction f = parseSum(expr, limit);
		
		// проверяем конец выражения
		Token token;
		if ( parseToken(token, expr, limit) != tok_end )
		{
			throw MathParserError("unexpected token (" + token.typeString() + "), end of string expected");
		}
		
		return f;
	}
	
	/**
	* Парсинг выражения
	* Если выражение содержит синтаксическую ошибку, то
	* генерируется исключение
	* @param expr выражение для парсинга
	* @return математическая фунция
	*/
	MathFunction MathParser::parse(const char *expr) const
	{
		return parse(expr, strlen(expr));
	}
	
	/**
	* Парсинг выражения
	* Если выражение содержит синтаксическую ошибку, то
	* генерируется исключение
	* @param expr выражение для парсинга
	* @return математическая фунция
	*/
	MathFunction MathParser::parse(const std::string &expr) const
	{
		return parse(expr.c_str(), expr.length());
	}
}
