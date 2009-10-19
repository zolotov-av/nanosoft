#include <nanosoft/mysql.h>
#include <mysql/mysql.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

namespace nanosoft
{
	MYSQL *mysql_init_struct = MySQL::init();
	
	MYSQL * MySQL::init()
	{
		MYSQL *m = mysql_init(0);
		if ( m ) return m;
		fprintf(stderr, "MySQL initialization fault\n");
		return 0;
	}
	
	/**
	* Конструктор
	*/
	MySQL::MySQL(): conn(0)
	{
	}
	
	/**
	* Деструктор
	*/
	MySQL::~MySQL()
	{
		close();
	}
	
	/**
	* Инициализация потока
	*/
	void MySQL::threadInit()
	{
		mysql_thread_init();
	}
	
	/**
	* Финализация потока
	*/
	void MySQL::threadEnd()
	{
		mysql_thread_end();
	}
	
	/**
	* Соединение сервером
	*
	* В случае неудачи выводит в stderr сообщение об ошибке и возращает FALSE
	*
	* @param host хост
	* @param database имя БД к которой подключаемся
	* @param user пользователь
	* @param password пароль
	* @param port порт
	* @return TRUE - соединение установлено, FALSE ошибка соединения
	*/
	bool MySQL::connect(const std::string &host, const std::string &database, const std::string &user, const std::string &password, int port)
	{
		conn = mysql_real_connect(mysql_init_struct, host.c_str(), user.c_str(), password.c_str(), database.c_str(), port, 0, 0);
		if ( conn ) return true;
		fprintf(stderr, "[MySQL] connection fault\n");
		return false;
	}
	
	/**
	* Соединение сервером
	*
	* В случае неудачи выводит в stderr сообщение об ошибке и возращает FALSE
	*
	* @param sock путь к Unix-сокету
	* @param database имя БД к которой подключаемся
	* @param user пользователь
	* @param password пароль
	* @return TRUE - соединение установлено, FALSE ошибка соединения
	*/
	bool MySQL::connectUnix(const std::string &sock, const std::string &database, const std::string &user, const std::string &password)
	{
		conn = mysql_real_connect(mysql_init_struct, 0, user.c_str(), password.c_str(), database.c_str(), 0, sock.c_str(), 0);
		if ( conn ) return true;
		fprintf(stderr, "[MySQL] connection fault\n");
		return false;
	}
	
	/**
	* Экранировать строку
	*/
	std::string MySQL::escape(const std::string &text)
	{
		char *buf = new char[text.length() * 2 + 1];
		size_t len = mysql_real_escape_string(conn, buf, text.c_str(), text.length());
		return std::string(buf, len);
	}
	
	/**
	* Экранировать строку и заключить её в кавычки
	*/
	std::string MySQL::quote(const std::string &text)
	{
		return '"' + text + '"';
	}
	
	/**
	* Выполнить произвольный SQL-запрос
	*/
	MySQL::result MySQL::query(const char *sql, ...)
	{
		char *buf;
		va_list args;
		va_start(args, sql);
		int len = vasprintf(&buf, sql, args);
		va_end(args);
		int status = mysql_real_query(conn, buf, len);
		free(buf);
		if ( status ) {
			fprintf(stderr, "[MySQL] %s\n", mysql_error(conn));
			return 0;
		}
		
		MYSQL_RES *res = mysql_store_result(conn);
		if ( res || mysql_field_count(conn) > 0 ) {
			ResultSet *r = new ResultSet;
			r->res = res;
			r->field_count = mysql_field_count(conn);
			r->fields = mysql_fetch_fields(res);
			r->values = mysql_fetch_row(res);
			if ( r->values ) r->lengths = mysql_fetch_lengths(res);
			return r;
		}
		
		if ( mysql_errno(conn) ) {
			fprintf(stderr, "[MySQL] %s\n", mysql_error(conn));
		}
		
		return 0;
	}
	
	/**
	* Закрыть соединение с сервером
	*/
	void MySQL::close()
	{
		if ( conn )
		{
			mysql_close(conn);
			conn = 0;
		}
	}
	
	void MySQL::result::next()
	{
		if ( res && res->res ) {
			res->values = mysql_fetch_row(res->res);
			if ( res->values ) res->lengths = mysql_fetch_lengths(res->res);
		}
	}
	
	int MySQL::result::indexOf(const char *name)
	{
		MYSQL_FIELD *p = res->fields;
		for(size_t i = 0; i < res->field_count; i++, p++)
		{
			if ( strcmp(name, p->name) == 0 ) return i;
		}
		fprintf(stderr, "[MySQL] field not found: %s\n", name);
		return 0;
	}
	
	std::string MySQL::result::operator [] (size_t num)
	{
		return std::string(res->values[num], res->lengths[num]);
	}
	
	std::string MySQL::result::operator [] (const char *name)
	{
		int num = indexOf(name);
		return std::string(res->values[num], res->lengths[num]);
	}
	
	bool MySQL::result::isNull(size_t num)
	{
		return res->values[num] != 0;
	}
	
	bool MySQL::result::isNull(const char *name)
	{
		int num = indexOf(name);
		return res->values[num] != 0;
	}
	
	/**
	* Удалить набор данных
	*/
	void MySQL::result::free()
	{
		mysql_free_result(res->res);
	}
}
