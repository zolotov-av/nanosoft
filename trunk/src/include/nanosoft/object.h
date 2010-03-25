#ifndef NANOSOFT_OBJECT_H
#define NANOSOFT_OBJECT_H

#include <stdint.h>
#include <sys/types.h>

namespace nanosoft
{
	/**
	* Базовый класс для объектов в счетчиком ссылок
	*/
	class Object
	{
	public:
		/**
		* Конструктор
		*/
		Object();
		
		/**
		* Деструктор
		*/
		virtual ~Object();
		
		/**
		* Заблокировать объект
		*/
		void lock();
		
		/**
		* Освободить объект
		*/
		void release();
		
	protected:
		/**
		* Обработка обнуления счетчика ссылок
		*
		* По умолчанию удаляет объект
		*/
		virtual void onFree();
		
	private:
		/**
		* Счетчик ссылок
		*/
		int ref_count;
		
		/**
		* Конструктор копий
		*
		* Не ищите его реализации, его нет и не надо.
		* Просто блокируем конкструктор копий по умолчанию
		*/
		Object(const Object &);
		
		/**
		* Оператор присваивания
		*
		* Блокируем аналогично конструктору копий
		*/
		Object& operator = (const Object &);
	};
	
	template <class type>
	class ptr
	{
	private:
		type *p;
	public:
		ptr() {
			p = 0;
		}
		
		ptr(type *obj) {
			p = obj;
			if ( p ) p->lock();
		}
		
		ptr(const ptr<type> &obj) {
			p = obj.p;
			if ( p ) p->lock();
		}
		
		~ptr() {
			if ( p ) p->release();
		}
		
		ptr<type>& operator = (type *obj) {
			if ( p != obj )
			{
				if ( p ) p->release();
				p = obj;
				if ( p ) p->lock();
			}
			return *this;
		}
		
		ptr<type>& operator = (const ptr<type> &obj) {
			if ( p != obj.p )
			{
				if ( p ) p->release();
				p = obj.p;
				if ( p ) p->lock();
			}
		}
		
		type* operator -> () {
			return p;
		}
		
		const type* operator -> () const {
			return p;
		}
	};
}

#endif // NANOSOFT_OBJECT_H
