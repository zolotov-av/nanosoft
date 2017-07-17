
#include <nanosoft/hash.h>

namespace nanosoft
{
	/**
	* Хеш-функция для строки
	*/
	size_t hash(const char *p)
	{
		size_t h = 0;
		size_t m = 1;
		while ( *p )
		{
			h += *p * m;
			m *= 31;
			p++;
		}
		return h;
	}
}
