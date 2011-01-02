#ifndef NANOSOFT_HASH_H
#define NANOSOFT_HASH_H

#include <nanosoft/ring.h>

namespace nanosoft
{
	/**
	* Хеш-функция для строки
	*/
	size_t hash(const char *p);
	
	/**
	* Класс хеш-таблицы
	*/
	template <class key, class type>
	class hashtab
	{
	private:
		/**
		* Размер таблицы - число ячеек в таблице
		*/
		size_t size;
		
		typedef neighbor<type> *item_t;
		
		/**
		* Ячейки хеш-таблицы
		* Все дубликаты и коллизии собраются в кольцо
		*/
		item_t *items;
	public:
		/**
		* Конструктор
		*/
		hashtab(size_t sz): size(sz) { items = new item_t[sz]; }
		
		/**
		* Деструктор
		*/
		~hashtab() { delete items; }
		
		/**
		* Добавить элемент в таблицу
		*/
		void add(key id, neighbor<type> *item);
		
		/**
		* Удалить элемент из таблицы
		*/
		void remove(key id, neighbor<type> *item);
		
		/**
		* Найти элемент в таблице
		*/
		neighbor<type>* lookup(key id);
	};
	
	template <class key, class type>
	void hashtab<key, type>::add(key id, neighbor<type> *item)
	{
		size_t h = hash(id) % size;
		if ( items[h] )
		{
			items[h]->attach(item);
		}
		else
		{
			item->detach();
			items[h] = item;
		}
	}
	
	template <class key, class type>
	void hashtab<key, type>::remove(key id, neighbor<type> *item)
	{
		size_t h = hash(id) % size;
		if ( items[h] )
		{
			if ( items[h] == item && item->getNext() == item ) items[h] = 0;
			else
			{
				if ( items[h] == item ) items[h] = items[h]->getNext();
				item->detach();
			}
		}
	}
	
	template <class key, class type>
	neighbor<type>* hashtab<key, type>::lookup(key id)
	{
		return items[ hash(id) % size ];
	}
}

#endif // NANOSOFT_HASH_H
