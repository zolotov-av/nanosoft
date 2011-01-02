#ifndef NANOSOFT_STRING_H
#define NANOSOFT_STRING_H

#include <sys/types.h>
#include <string.h>

namespace nanosoft
{
	template <class type>
	class neighbor;
	
	/**
	* Класс кольцо - вдухсвязный список объектов
	*/
	template <class type>
	class ring
	{
	private:
		neighbor<type> *first;
	public:
		ring(): first(0) { }
		neighbor<type> *getFirst() const { return first; }
		void add(neighbor<type> *item);
		void remove(neighbor<type> *item);
	};
	
	/**
	* Класс сосед - описывает элемент кольца
	*/
	template <class type>
	class neighbor
	{
		friend class ring<type>;
	private:
		type *obj;
		ring<type> *owner;
		neighbor<type> *next;
		neighbor<type> *prev;
	public:
		neighbor(type *o): obj(o), owner(0), next(0), prev(0) { }
		~neighbor() { if ( owner) owner->remove(this); }
		type* getObject() const { return obj; }
		neighbor<type>* getNext() const { return next; }
		neighbor<type>* getPrev() const { return prev; }
	};
	
	template <class type>
	void ring<type>::add(neighbor<type> *item)
	{
		if ( item->owner ) item->owner->remove(item);
		item->owner = this;
		
		if ( first == 0 )
		{
			first = item;
			first->next = item;
			first->prev = item;
		}
		else
		{
			item->next = first->next;
			item->prev = first;
			
			item->prev->next = item;
			item->next->prev = item;
		}
	}
	
	template <class type>
	void ring<type>::remove(neighbor<type> *item)
	{
		if ( item->owner != this ) return;
		
		if ( item->next == item )
		{
			item->owner->first = 0;
		}
		else
		{
			item->prev->next = item->next;
			item->next->prev = item->prev;
			item->owner->first = item->next;
		}
		
		item->owner = 0;
		item->next = 0;
		item->prev = 0;
	}
}

#endif // NANOSOFT_STRING_H
