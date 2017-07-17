#ifndef NANOSOFT_MUTEX_H
#define NANOSOFT_MUTEX_H

#ifdef USE_PTHREAD
#include <pthread.h>
#endif

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
#ifdef USE_PTHREAD
		/**
		* Mutex для thread-safe доступа к БД
		*/
		pthread_mutex_t mutex;
#endif
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
