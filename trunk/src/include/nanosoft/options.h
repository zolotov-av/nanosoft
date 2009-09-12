#ifndef NANO_OPTIONS_H
#define NANO_OPTIONS_H

#include <sys/types.h>

namespace nanosoft
{
	
	class options
	{
	private:
		
		struct option_t
		{
			char *key;
			char *value;
		};
		
		/**
		* Список значений
		*/
		option_t *items;
		
		int allocBy;
		
		/**
		* Размер массива
		*/
		int size;
		
		/**
		* Число опций
		*/
		int count;
		
		options(const options &) { }
		options& operator = (const options &) { }
	public:
		options(int allocBy = 16);
		~options();
		void clear();
		const char *read(const char *key, const char *defaultValue = 0);
		bool write(const char *key, const char *value);
		void remove(const char *key);
		int getCount();
		const char * keyAt(int pos);
		const char * valueAt(int pos);
		
		const char * operator[] (const char *key);
	};
	
	inline int options::getCount()
	{
		return count;
	}
	
	inline const char * options::keyAt(int pos)
	{
		return items[pos].key;
	}
	
	inline const char * options::valueAt(int pos)
	{
		return items[pos].value;
	}
	
	inline const char * options::operator [] (const char *key)
	{
		return read(key, 0);
	}
}

#endif // NANO_OPTIONS_H
