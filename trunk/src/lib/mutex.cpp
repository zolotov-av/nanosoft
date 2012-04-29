#include <nanosoft/mutex.h>
#include <nanosoft/error.h>

#ifdef USE_PTHREAD
#include <pthread.h>
#endif

namespace nanosoft
{
	/**
	* Конструктор
	*/
	Mutex::Mutex()
	{
#ifdef USE_PTHREAD
		stdcheck( pthread_mutex_init(&mutex, 0) == 0 );
#endif
	}
	
	/**
	* Деструктор
	*/
	Mutex::~Mutex()
	{
#ifdef USE_PTHREAD
		stdcheck( pthread_mutex_destroy(&mutex) == 0 );
#endif
	}
	
	/**
	* Получить монопольный доступ к БД
	*/
	bool Mutex::lock()
	{
#ifdef USE_PTHREAD
		return pthread_mutex_lock(&mutex) == 0;
#else
		return 1;
#endif
	}
	
	/**
	* Освободить БД
	*/
	void Mutex::unlock()
	{
#ifdef USE_PTHREAD
		stdcheck( pthread_mutex_unlock(&mutex) == 0 );
#endif
	}
}
