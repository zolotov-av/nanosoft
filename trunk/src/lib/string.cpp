
#include <nanosoft/string.h>
#include <cstring>
#include <cstdlib>
#include <ctype.h>

namespace nanosoft
{
	/**
	* Записать в буфер dest строку src
	* Аналог strncpy всегда завершает строку нулём
	* @param dest куда копировать
	* @param dest_len размер буфера
	* @param src исходная строка
	* @return TRUE строка скопированна полностью
	*/
	bool strset(char *dest, size_t dest_len, const char *src)
	{
		char *limit = dest + dest_len - 1;
		while ( *src && dest < limit ) *dest++ = *src++;
		*dest = 0;
		return dest < limit;
	}
	
	/**
	* Записать в буфер dest строку trim(src)
	* Обрезает пробелы в строке src и копирует строку в dest
	* Всегда завершает строку нулём
	* @param dest куда копировать
	* @param dest_len размер буфера
	* @param src исходная строка
	* @return TRUE строка скопированна полностью
	*/
	bool strtim(char *dest, size_t dest_len, const char *src)
	{
		char *limit = dest + dest_len - 1;
		while ( *src == ' ' || *src == '\n' || *src == '\r' || *src == '\t' ) src++;
		char *p = dest;
		while ( *src && dest < limit )
		{
			if ( *src != ' ' && *src != '\n' && *src != '\r' && *src != '\t' ) p = dest;
			*dest++ = *src++;
		}
		p[1] = 0;
		return dest < limit;
	}
}
