
#include <nanosoft/object.h>

namespace nanosoft
{
	/**
	* Конструктор
	*/
	Object::Object()
	{
		ref_count = 0;
	}
	
	/**
	* Деструктор
	*/
	Object::~Object()
	{
	}
	
	/**
	* Заблокировать объект
	*/
	void Object::lock()
	{
		__asm__ __volatile__(
			"lock incl %0"
			:"=m" (ref_count)
			:"m" (ref_count)
		);
	}
	
	/**
	* Освободить объект
	*/
	void Object::release()
	{
		unsigned char c;
		__asm__ __volatile__(
			"lock decl %0; sete %1"
			:"=m" (ref_count), "=qm" (c)
			:"m" (ref_count)
		);
		if ( c != 0 ) onFree();
	}
	
	/**
	* Обработка обнуления счетчика ссылок
	*
	* По умолчанию удаляет объект
	*/
	void Object::onFree()
	{
		delete this;
	}
}
