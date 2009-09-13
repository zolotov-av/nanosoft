#ifndef NANOSOFT_STRING_H
#define NANOSOFT_STRING_H

#include <sys/types.h>

namespace nanosoft
{
	
	class cstring
	{
	private:
		char *data;
		size_t len;
		void setcopy(const char *chars, size_t nchars);
		void setcopy(const cstring &s, int offset, int nchars);
	public:
		cstring();
		cstring(const char *chars);
		cstring(const char *chars, size_t len);
		cstring(const cstring &s);
		cstring(const cstring &s, int start, int nchars);
		~cstring();
		
		size_t length() const;
		const char * c_str() const;
		operator const char * () const;
		cstring & operator = (const char *chars);
		cstring & operator = (const cstring &s);
		void copy(const cstring &s, int start, int chars);
		void trim(const cstring &s);
	};
	
	inline cstring::cstring(): data(0), len(0)
	{
	}
	
	inline size_t cstring::length() const
	{
		return len;
	}
	
	inline const char * cstring::c_str() const
	{
		return data;
	}
	
	inline cstring::operator const char * () const
	{
		return data;
	}
	
	inline cstring substr(const cstring &s, int offset, int nchars)
	{
		return cstring(s, offset, nchars);
	}
	
}

#endif // NANOSOFT_STRING_H
