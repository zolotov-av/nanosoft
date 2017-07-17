
#include <nanosoft/options.h>
#include <cstdlib>
#include <cstring>

namespace nanosoft
{
	
	options::options(int _allocBy):
		items(0),
		allocBy(_allocBy),
		size(0),
		count(0)
	{
		
	}
	
	options::~options()
	{
		clear();
	}
	
	void options::clear()
	{
		if ( items )
		{
			option_t *end = items + count;
			for(option_t *p = items; p < end; p++)
			{
				if ( p->value[0] )
				{
					free(p->key);
					free(p->value);
				}
			}
			free(items);
			items = 0;
			size = 0;
			count = 0;
		}
	}
	
	const char * options::read(const char *key, const char *defaultValue)
	{
		option_t *end = items + count;
		for(option_t *p = items; p < end; p++)
		{
			if ( strcmp(p->key, key) == 0 ) return p->value;
		}
		return defaultValue;
	}
	
	bool options::write(const char *key, const char *value)
	{
		option_t *end = items + count;
		for(option_t *p = items; p < end; p++)
		{
			if ( strcmp(p->key, key) == 0 )
			{
				char *dup = strdup(value);
				if ( dup == 0 ) return 0;
				free(p->value);
				p->value = dup;
				return 1;
			}
		}
		
		if ( count == size )
		{
			int new_size = size + allocBy;
			option_t *new_items = static_cast<option_t*>(realloc(items, new_size * sizeof(option_t)));
			if ( new_items == 0 ) return 0;
			items = new_items;
			size = new_size;
		}
		
		char *k = strdup(key);
		char *v = strdup(value);
		if ( k == 0 || v == 0 )
		{
			free(k);
			free(v);
			return 0;
		}
		items[count].key = k;
		items[count].value = v;
		count++;
		return 1;
	}
	
	void options::remove(const char *key)
	{
		option_t *end = items + count;
		for(option_t *p = items; p < end; p++)
		{
			if ( strcmp(p->key, key) == 0 )
			{
				free(p->key);
				free(p->value);
				
				if ( p + 1 < end)
				{
					p->key = end[-1].key;
					p->value = end[-1].value;
				}
				
				count--;
				
				return;
			}
		}
	}
}
