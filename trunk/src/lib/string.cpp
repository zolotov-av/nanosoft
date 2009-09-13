
#include <nanosoft/string.h>
#include <cstring>
#include <cstdlib>
#include <ctype.h>

namespace nanosoft
{
	void cstring::setcopy(const char *chars, size_t nchars)
	{
		if ( chars && nchars > 0 )
		{
			len = nchars;
			data = static_cast<char *>( malloc(len + 1) );
			memcpy(data, chars, len);
			data[len] = 0;
		}
		else
		{
			data = 0;
			len = 0;
		}
	}
	
	void cstring::setcopy(const cstring &s, int offset, int nchars)
	{
		if ( nchars <= 0 || s.len == 0 )
		{
			setcopy(0, 0);
			return;
		}
		
		if ( offset < 0 )
		{
			nchars += offset;
			if ( nchars <= 0 )
			{
				setcopy(0, 0);
				return;
			}
			offset = 0;
		}
		
		int rem = s.len - offset;
		if ( rem <= 0 )
		{
			setcopy(0, 0);
			return;
		}
		
		if ( rem < nchars ) nchars = rem;
		
		setcopy(s.data + offset, nchars);
	}
	
	cstring::cstring(const char *chars)
	{
		setcopy(chars, strlen(chars));
	}
	
	cstring::cstring(const char *chars, size_t nchars)
	{
		setcopy(chars, nchars);
	}
	
	cstring::cstring(const cstring &s)
	{
		setcopy(s.data, s.len);
	}
	
	cstring::cstring(const cstring &s, int offset, int nchars)
	{
		setcopy(s, offset, nchars);
	}
	
	cstring::~cstring()
	{
		free(data);
	}
	
	void cstring::copy(const cstring &s, int offset, int nchars)
	{
		char *old = data;
		setcopy(s, offset, nchars);
		free(old);
	}
	
	void cstring::trim(const cstring &s)
	{
		char *old = data;
		const char *left = s.data;
		const char *right = s.data + s.len - 1;
		while ( isspace(*left) && left < right ) left++;
		while ( isspace(*right) && left < right ) right--;
		setcopy(left, right - left + 1);
		free(old);
	}
	
	cstring & cstring::operator = (const char *chars)
	{
		free(data);
		setcopy(chars, strlen(chars));
		return *this;
	}
	
	cstring & cstring::operator = (const cstring &s)
	{
		if ( &s != this )
		{
			free(data);
			setcopy(s.data, s.len);
		}
		return *this;
	}
	
}
