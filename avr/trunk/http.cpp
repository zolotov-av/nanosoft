
#include "http.h"

const char *http_p;
const char *http_limit;

unsigned char char2hex(char ch)
{
	if ( ch >= '0' && ch <= '9' ) return ch - '0';
	if ( ch >= 'A' && ch <= 'F' ) return ch - 'A' + 10;
	if ( ch >= 'a' && ch <= 'f' ) return ch - 'a' + 10;
	return 0;
}

bool parse_char(char *ch)
{
	if ( http_p >= http_limit ) return false;
	if ( *http_p == '%' )
	{
		if ( http_p + 2 <= http_limit )
		{
			*ch = char2hex(http_p[1]) * 16 + char2hex(http_p[2]);
			http_p += 3;
			return true;
		}
		return false;
	}
	if ( *http_p == '+' )
	{
		*ch = ' ';
		http_p++;
		return true;
	}
	*ch = *http_p++;
	return true;
}

bool parse_param()
{
	static char pname_buf[32];
	static char pvalue_buf[32];
	char *pname = pname_buf;
	char *pvalue = pvalue_buf;
	char *pname_limit = pname_buf + sizeof(pname_buf) / sizeof(pname_buf[0])-1;
	char *pvalue_limit = pvalue_buf + sizeof(pvalue_buf) / sizeof(pvalue_buf[0])-1;
	char ch;
	while ( http_p < http_limit && *http_p != '&' )
	{
		if ( *http_p == '=' )
		{
			http_p++;
			*pname = 0;
			while ( http_p < http_limit && *http_p != '&' )
			{
				if ( parse_char(&ch) )
				{
					if ( pvalue < pvalue_limit ) *pvalue++ = ch;
				}
				else break;
			}
			*pvalue = 0;
			on_parse_param(pname_buf, pvalue_buf);
			if ( http_p < http_limit )
			{
				http_p++;
				return 1;
			}
			return 0;
		}
		if ( parse_char(&ch) )
		{
			if ( pname < pname_limit ) *pname++ = ch;
		}
		else break;
	}
	*pname = 0;
	*pvalue = 0;
	on_parse_param(pname_buf, pvalue_buf);
	if ( http_p < http_limit )
	{
		http_p++;
		return 1;
	}
	return 0;
}

void parse_request(const char *buf, int len)
{
	http_p = buf;
	http_limit = buf + len;
	while ( parse_param() ) ;
}
