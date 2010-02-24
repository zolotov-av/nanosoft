
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
		virtual MathFunction parse(MathParser *p, const char *&expr, const char *limit) = 0;
	};
	
	/**
	* Класс парсера переменных
	*/
	class MathVarParser: public MathHelperParser
	{
	private:
		/**
		* Переменная
		*/
		MathVar var;
	public:
		/**
		* Конструктор парсера переменной
		*/
		MathVarParser(const MathVar &v): var(v) { }
		
		/**
		* Парсер переменной
		*/
		MathFunction parse(MathParser *p, const char *&expr, const char *limit) {
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
		MathFunction parse(MathParser *p, const char *&expr, const char *limit)
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
		MathFunction parse(MathParser *p, const char *&expr, const char *limit)
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
	* Конструктор парсера
	*/
	MathParser::MathParser()
	{
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
	void MathParser::set(const char *name, const MathVar &var)
	{
		names_t::iterator iter = names.find(name);
		if ( iter != names.end() ) delete iter->second;
		names[name] = new MathVarParser(var);
	}
	
	/**
	* Добавить функцию одной переменной
	*/
	void MathParser::set(const char *name, const MathFunctionX f)
	{
		names_t::iterator iter = names.find(name);
		if ( iter != names.end() ) delete iter->second;
		names[name] = new MathFunctionXParser(f);
	}
	
	/**
	* Добавить функцию двух переменных
	*/
	void MathParser::set(const char *name, const MathFunctionXY f)
	{
		names_t::iterator iter = names.find(name);
		if ( iter != names.end() ) delete iter->second;
		names[name] = new MathFunctionXYParser(f);
	}
	
	/**
	* Проверить зарегистрированное ли имя
	*/
	bool MathParser::isset(const char *name)
	{
		names_t::iterator iter = names.find(name);
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
	* Парсинг лексемы числа
	* @param token стуктура принимающая описание лексемы
	* @param expr выражение
	* @param limit ограничение строки
	* @return тип лексемы
	*/
	MathParser::token_type parseNumber(MathParser::Token &token, const char *&expr, const char *limit)
	{
		const char *p = expr + 1;
		while ( *p >= '0' && *p <= '9' ) p++;
		if ( *p == '.' )
		{
			p++;
			while ( *p >= '0' && *p <= '9' ) p++;
		}
		int len = p - expr;
		token.begin = expr;
		token.limit = p;
		token.value = std::string(expr, len);
		expr = p;
		return token.type = MathParser::tok_number;
	}
	
	/**
	* Парсинг лексемы идентификатора (переменной, функции)
	* @param token стуктура принимающая описание лексемы
	* @param expr выражение
	* @param limit ограничение строки
	* @return тип лексемы
	*/
	MathParser::token_type parseName(MathParser::Token &token, const char *&expr, const char *limit)
	{
		const char *p = expr + 1;
		while ( *p >= 'A' && *p <= 'Z'
			|| *p >= 'a' && *p <= 'z'
			|| *p >= '0' && *p <= '9'
			|| *p == '_' ) p++;
		int len = p - expr;
		token.begin = expr;
		token.limit = p;
		token.value = std::string(expr, len);
		expr = p;
		return token.type = MathParser::tok_name;
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
	* Парсинг подвыражения в скобках
	* Если выражение содержит синтаксическую ошибку, то
	* генерируется исключение
	* @param expr выражение для парсинга
	* @param limit ограничение
	* @return математическая фунция
	*/
	MathFunction MathParser::parseSubExpr(const char *&expr, const char *limit)
	{
		Token token;
		
		if ( parseToken(token, expr, limit) != tok_open )
		{
			throw MathParserError("expected subexpression");
		}
		
		MathFunction f = parseSum(expr, limit);
		
		if ( parseToken(token, expr, limit) != tok_close )
		{
			throw MathParserError("expected end of expression");
		}
		return f;
	}
	
	/**
	* Парсинг переменной и вызова функций
	* Если выражение содержит синтаксическую ошибку, то
	* генерируется исключение
	* @param expr выражение для парсинга
	* @param limit ограничение
	* @return математическая фунция
	*/
	MathFunction MathParser::parseNamedObject(const Token &token, const char *&expr, const char *limit)
	{
		names_t::iterator iter = names.find(token.value);
		if ( iter != names.end() ) {
			return iter->second->parse(this, expr, limit);
		}
		throw MathParserError("unknown variable or function: " + token.value);
	}
	
	/**
	* Парсинг атомарного выражения
	* Если выражение содержит синтаксическую ошибку, то
	* генерируется исключение
	* @param expr выражение для парсинга
	* @param limit ограничение
	* @return математическая фунция
	*/
	MathFunction MathParser::parseAtom(const char *&expr, const char *limit)
	{
		Token token;
		switch ( parseToken(token, expr, limit) )
		{
		case tok_end: throw MathParserError("unexpected end of expression");
		case tok_number: return atof(token.value.c_str());
		case tok_name: return parseNamedObject(token, expr, limit);
		case tok_open: { expr = token.begin; return parseSubExpr(expr, limit); }
		default:
			MathParserError("unexpected");
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
	MathFunction MathParser::parseMult(const char *&expr, const char *limit)
	{
		Token token;
		token_type op;
		MathFunction mult = parseAtom(expr, limit);
		while ( 1 )
		{
			switch ( parseToken(token, expr, limit) )
			{
			case tok_end:
				return mult;
			case tok_plus:
			case tok_minus:
			case tok_close:
			case tok_comma:
				expr = token.begin;
				return mult;
			case tok_mult:
			case tok_div:
				op = token.type;
				break;
			default:
				throw MathParserError("operator expected: ('*', '/')");
			}
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
	MathFunction MathParser::parseSum(const char *&expr, const char *limit)
	{
		Token token;
		token_type op = parseToken(token, expr, limit);
		if ( op != tok_minus && op != tok_plus ) expr = token.begin;
		MathFunction sum = parseMult(expr, limit);
		if ( op == tok_minus ) sum = - sum;
		while ( 1 )
		{
			switch ( parseToken(token, expr, limit) )
			{
			case tok_end:
				return sum;
			case tok_close:
			case tok_comma:
				expr = token.begin;
				return sum;
			case tok_plus:
			case tok_minus:
				op = token.type;
				break;
			default:
				throw MathParserError("operator expected: ('+', '-')");
			}
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
	MathFunction MathParser::parse(const char *expr, size_t len)
	{
		return parseSum(expr, expr + len);
	}
	
	/**
	* Парсинг выражения
	* Если выражение содержит синтаксическую ошибку, то
	* генерируется исключение
	* @param expr выражение для парсинга
	* @return математическая фунция
	*/
	MathFunction MathParser::parse(const char *expr)
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
	MathFunction MathParser::parse(const std::string &expr)
	{
		return parse(expr.c_str(), expr.length());
	}
}
