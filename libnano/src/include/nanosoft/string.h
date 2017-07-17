#ifndef NANOSOFT_STRING_H
#define NANOSOFT_STRING_H

#include <sys/types.h>
#include <string.h>

namespace nanosoft
{
	/**
	* Записать в буфер dest строку src
	* Аналог strncpy, но всегда завершает строку нулём
	* @param dest куда копировать
	* @param dest_len размер буфера
	* @param src исходная строка
	* @return TRUE строка скопированна полностью
	*/
	bool strset(char *dest, size_t dest_len, const char *src);
	
	/**
	* Записать в буфер dest строку trim(src)
	* Обрезает пробелы в строке src и копирует строку в dest
	* Всегда завершает строку нулём
	* @param dest куда копировать
	* @param dest_len размер буфера
	* @param src исходная строка
	* @return TRUE строка скопированна полностью
	*/
	bool strtim(char *dest, size_t dest_len, const char *src);
	
	/**
	* Строковый буфер
	*/
	template <size_t size>
	class strbuf
	{
	private:
		char buf[size];
	public:
		strbuf() { buf[0] = 0; }
		strbuf(const char *s) { strset(buf, size, s); }
		strbuf(const strbuf<size> &s) { strset(buf, size, s.buf); }
		void operator = (const char *s) { strset(buf, size, s); }
		const char * c_str() const { return buf; }
		char * ptr() { return buf; }
		size_t bufsz() { return size; }
	};
	
	/**
	* Строковый буфер с автотримом
	*/
	template <size_t size>
	class tstrbuf
	{
	private:
		char buf[size];
	public:
		tstrbuf() { buf[0] = 0; }
		tstrbuf(const char *s) { strtim(buf, size, s); }
		tstrbuf(const tstrbuf<size> &s) { strtim(buf, size, s.buf); }
		void operator = (const char *s) { strtim(buf, size, s); }
		const char * c_str() const { return buf; }
		char * ptr() { return buf; }
		size_t bufsz() { return size; }
	};
	
	template <size_t size>
	inline bool operator == (const char *s1, const strbuf<size> &s2)
	{
		return strcmp(s1, s2.c_str()) == 0;
	}
	
	template <size_t size>
	inline bool operator == (const char *s1, const tstrbuf<size> &s2)
	{
		return strcmp(s1, s2.c_str()) == 0;
	}
	
	template <size_t size>
	inline bool operator == (const strbuf<size> &s1, const char *s2)
	{
		return strcmp(s1.c_str(), s2) == 0;
	}
	
	template <size_t size1, size_t size2>
	inline bool operator == (const strbuf<size1> &s1, const tstrbuf<size2> &s2)
	{
		return strcmp(s1.c_str(), s2.c_str()) == 0;
	}
	
	template <size_t size1, size_t size2>
	inline bool operator == (const strbuf<size1> &s1, const strbuf<size2> &s2)
	{
		return strcmp(s1.c_str(), s2.c_str()) == 0;
	}
	
	template <size_t size>
	inline bool operator == (const tstrbuf<size> &s1, const char *s2)
	{
		return strcmp(s1.c_str(), s2) == 0;
	}
	
	template <size_t size1, size_t size2>
	inline bool operator == (const tstrbuf<size1> &s1, const strbuf<size2> &s2)
	{
		return strcmp(s1.c_str(), s2.c_str()) == 0;
	}
	
	template <size_t size1, size_t size2>
	inline bool operator == (const tstrbuf<size1> &s1, const tstrbuf<size2> &s2)
	{
		return strcmp(s1.c_str(), s2.c_str()) == 0;
	}
	
	/////
	
	template <size_t size>
	inline bool operator != (const char *s1, const strbuf<size> &s2)
	{
		return strcmp(s1, s2.c_str()) != 0;
	}
	
	template <size_t size>
	inline bool operator != (const char *s1, const tstrbuf<size> &s2)
	{
		return strcmp(s1, s2.c_str()) != 0;
	}
	
	template <size_t size>
	inline bool operator != (const strbuf<size> &s1, const char *s2)
	{
		return strcmp(s1.c_str(), s2) != 0;
	}
	
	template <size_t size1, size_t size2>
	inline bool operator != (const strbuf<size1> &s1, const tstrbuf<size2> &s2)
	{
		return strcmp(s1.c_str(), s2.c_str()) != 0;
	}
	
	template <size_t size1, size_t size2>
	inline bool operator != (const strbuf<size1> &s1, const strbuf<size2> &s2)
	{
		return strcmp(s1.c_str(), s2.c_str()) != 0;
	}
	
	template <size_t size>
	inline bool operator != (const tstrbuf<size> &s1, const char *s2)
	{
		return strcmp(s1.c_str(), s2) != 0;
	}
	
	template <size_t size1, size_t size2>
	inline bool operator != (const tstrbuf<size1> &s1, const strbuf<size2> &s2)
	{
		return strcmp(s1.c_str(), s2.c_str()) != 0;
	}
	
	template <size_t size1, size_t size2>
	inline bool operator != (const tstrbuf<size1> &s1, const tstrbuf<size2> &s2)
	{
		return strcmp(s1.c_str(), s2.c_str()) != 0;
	}
	
	/////
	
	template <size_t size>
	inline bool operator >= (const char *s1, const strbuf<size> &s2)
	{
		return strcmp(s1, s2.c_str()) >= 0;
	}
	
	template <size_t size>
	inline bool operator >= (const char *s1, const tstrbuf<size> &s2)
	{
		return strcmp(s1, s2.c_str()) >= 0;
	}
	
	template <size_t size>
	inline bool operator >= (const strbuf<size> &s1, const char *s2)
	{
		return strcmp(s1.c_str(), s2) >= 0;
	}
	
	template <size_t size1, size_t size2>
	inline bool operator >= (const strbuf<size1> &s1, const tstrbuf<size2> &s2)
	{
		return strcmp(s1.c_str(), s2.c_str()) >= 0;
	}
	
	template <size_t size1, size_t size2>
	inline bool operator >= (const strbuf<size1> &s1, const strbuf<size2> &s2)
	{
		return strcmp(s1.c_str(), s2.c_str()) >= 0;
	}
	
	template <size_t size>
	inline bool operator >= (const tstrbuf<size> &s1, const char *s2)
	{
		return strcmp(s1.c_str(), s2) >= 0;
	}
	
	template <size_t size1, size_t size2>
	inline bool operator >= (const tstrbuf<size1> &s1, const strbuf<size2> &s2)
	{
		return strcmp(s1.c_str(), s2.c_str()) >= 0;
	}
	
	template <size_t size1, size_t size2>
	inline bool operator >= (const tstrbuf<size1> &s1, const tstrbuf<size2> &s2)
	{
		return strcmp(s1.c_str(), s2.c_str()) >= 0;
	}
	
	/////
	
	template <size_t size>
	inline bool operator <= (const char *s1, const strbuf<size> &s2)
	{
		return strcmp(s1, s2.c_str()) <= 0;
	}
	
	template <size_t size>
	inline bool operator <= (const char *s1, const tstrbuf<size> &s2)
	{
		return strcmp(s1, s2.c_str()) <= 0;
	}
	
	template <size_t size>
	inline bool operator <= (const strbuf<size> &s1, const char *s2)
	{
		return strcmp(s1.c_str(), s2) <= 0;
	}
	
	template <size_t size1, size_t size2>
	inline bool operator <= (const strbuf<size1> &s1, const tstrbuf<size2> &s2)
	{
		return strcmp(s1.c_str(), s2.c_str()) <= 0;
	}
	
	template <size_t size1, size_t size2>
	inline bool operator <= (const strbuf<size1> &s1, const strbuf<size2> &s2)
	{
		return strcmp(s1.c_str(), s2.c_str()) <= 0;
	}
	
	template <size_t size>
	inline bool operator <= (const tstrbuf<size> &s1, const char *s2)
	{
		return strcmp(s1.c_str(), s2) <= 0;
	}
	
	template <size_t size1, size_t size2>
	inline bool operator <= (const tstrbuf<size1> &s1, const strbuf<size2> &s2)
	{
		return strcmp(s1.c_str(), s2.c_str()) <= 0;
	}
	
	template <size_t size1, size_t size2>
	inline bool operator <= (const tstrbuf<size1> &s1, const tstrbuf<size2> &s2)
	{
		return strcmp(s1.c_str(), s2.c_str()) <= 0;
	}
	
	/////
	
	template <size_t size>
	inline bool operator > (const char *s1, const strbuf<size> &s2)
	{
		return strcmp(s1, s2.c_str()) > 0;
	}
	
	template <size_t size>
	inline bool operator > (const char *s1, const tstrbuf<size> &s2)
	{
		return strcmp(s1, s2.c_str()) > 0;
	}
	
	template <size_t size>
	inline bool operator > (const strbuf<size> &s1, const char *s2)
	{
		return strcmp(s1.c_str(), s2) > 0;
	}
	
	template <size_t size1, size_t size2>
	inline bool operator > (const strbuf<size1> &s1, const tstrbuf<size2> &s2)
	{
		return strcmp(s1.c_str(), s2.c_str()) > 0;
	}
	
	template <size_t size1, size_t size2>
	inline bool operator > (const strbuf<size1> &s1, const strbuf<size2> &s2)
	{
		return strcmp(s1.c_str(), s2.c_str()) > 0;
	}
	
	template <size_t size>
	inline bool operator > (const tstrbuf<size> &s1, const char *s2)
	{
		return strcmp(s1.c_str(), s2) > 0;
	}
	
	template <size_t size1, size_t size2>
	inline bool operator > (const tstrbuf<size1> &s1, const strbuf<size2> &s2)
	{
		return strcmp(s1.c_str(), s2.c_str()) > 0;
	}
	
	template <size_t size1, size_t size2>
	inline bool operator > (const tstrbuf<size1> &s1, const tstrbuf<size2> &s2)
	{
		return strcmp(s1.c_str(), s2.c_str()) > 0;
	}
	
	/////
	
	template <size_t size>
	inline bool operator < (const char *s1, const strbuf<size> &s2)
	{
		return strcmp(s1, s2.c_str()) < 0;
	}
	
	template <size_t size>
	inline bool operator < (const char *s1, const tstrbuf<size> &s2)
	{
		return strcmp(s1, s2.c_str()) < 0;
	}
	
	template <size_t size>
	inline bool operator < (const strbuf<size> &s1, const char *s2)
	{
		return strcmp(s1.c_str(), s2) < 0;
	}
	
	template <size_t size1, size_t size2>
	inline bool operator < (const strbuf<size1> &s1, const strbuf<size2> &s2)
	{
		return strcmp(s1.c_str(), s2.c_str()) < 0;
	}
	
	template <size_t size1, size_t size2>
	inline bool operator < (const strbuf<size1> &s1, const tstrbuf<size2> &s2)
	{
		return strcmp(s1.c_str(), s2.c_str()) < 0;
	}
	
	template <size_t size>
	inline bool operator < (const tstrbuf<size> &s1, const char *s2)
	{
		return strcmp(s1.c_str(), s2) < 0;
	}
	
	template <size_t size1, size_t size2>
	inline bool operator < (const tstrbuf<size1> &s1, const strbuf<size2> &s2)
	{
		return strcmp(s1.c_str(), s2.c_str()) < 0;
	}
	
	template <size_t size1, size_t size2>
	inline bool operator < (const tstrbuf<size1> &s1, const tstrbuf<size2> &s2)
	{
		return strcmp(s1.c_str(), s2.c_str()) < 0;
	}
}

#endif // NANOSOFT_STRING_H
