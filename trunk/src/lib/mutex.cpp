#include <nanosoft/mutex.h>
#include <nanosoft/error.h>
#include <pthread.h>

namespace nanosoft
{
	/**
	* Конструктор
	*/
	Mutex::Mutex()
	{
		stdcheck( pthread_mutex_init(&mutex, 0) == 0 );
	}
	
	/**
	* Деструктор
	*/
	Mutex::~Mutex()
	{
		stdcheck( pthread_mutex_destroy(&mutex) == 0 );
	}
	
	/**
	* Получить монопольный доступ к БД
	*/
	void Mutex::lock()
	{
		stdcheck( pthread_mutex_lock(&mutex) == 0 );
	}
	
	/**
	* Освободить БД
	*/
	void Mutex::unlock()
	{
		stdcheck( pthread_mutex_unlock(&mutex) == 0 );
	}
}
