#ifndef NANOSOFT_MUTEX_H
#define NANOSOFT_MUTEX_H

#include <pthread.h>

namespace nanosoft
{
	/**
	* Mutex
	*
	* Класс-оболочка для pthread_mutex_t который автоматизирует часть функций
	*/
	class Mutex
	{
	protected:
		/**
		* Mutex для thread-safe доступа к БД
		*/
		pthread_mutex_t mutex;
		
	public:
		/**
		* Конструктор
		*/
		Mutex();
		
		/**
		* Деструктор
		*/
		~Mutex();
		
		/**
		* Получить монопольный доступ к БД
		*/
		bool lock();
		
		/**
		* Освободить БД
		*/
		void unlock();
	};
}

#endif // NANOSOFT_MUTEX_H
