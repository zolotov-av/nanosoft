#ifndef NANOSOFT_RING_H
#define NANOSOFT_RING_H

#include <sys/types.h>
#include <string.h>

namespace nanosoft
{
	/**
	* Класс сосед - описывает элемент кольца
	*/
	template <class type>
	class neighbor
	{
	private:
		type *obj;
		neighbor<type> *next;
		neighbor<type> *prev;
	public:
		/**
		* Конструктор
		*/
		neighbor(type *o): obj(o), next(this), prev(this) { }
		
		/**
		* Деструктор
		*/
		~neighbor() { detach(); }
		
		/**
		* Вернуть указатель на объект-родителя
		*/
		type* getObject() const { return obj; }
		
		/**
		* Вернуть следующего соседа
		*/
		neighbor<type>* getNext() const { return next; }
		
		/**
		* Вернуть предыдущего соседа
		*/
		neighbor<type>* getPrev() const { return prev; }
		
		/**
		* Присоединиться к другому соседу/цепочке
		*/
		void attachTo(neighbor<type> *item);
		
		/**
		* Присоединить к себе соседа
		*/
		void attach(neighbor<type> *item) { item->attachTo(this); }
		
		/**
		* Отсоединиться от цепочки
		*/
		void detach();
	};
	
	template <class type>
	void neighbor<type>::attachTo(neighbor<type> *item)
	{
		detach();
		if ( item == 0 ) return;
		
		this->next = item->next;
		this->prev = item;
		
		this->prev->next = this;
		this->next->prev = this;
	}
	
	template <class type>
	void neighbor<type>::detach()
	{
		if ( next == this ) return;
		
		this->prev->next = this->next;
		this->next->prev = this->prev;
		
		this->next = this;
		this->prev = this;
	}
}

#endif // NANOSOFT_RING_H
