#ifndef NANOSOFT_NETDAEMON_H
#define NANOSOFT_NETDAEMON_H

#include <sys/epoll.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <map>
#include <nanosoft/asyncstream.h>

/**
* Главный класс сетевого демона
*/
class NetDaemon
{
private:
	/**
	* Файловый дескриптор epoll
	*/
	int epoll;
	
	/**
	* Карта потоков (fd -> AsyncStream)
	*/
	std::map<int, AsyncStream*> streams;
	
	/**
	* Число потоков-воркеров
	*/
	int workerCount;
	
	/**
	* Список воркеров
	*/
	pthread_t *workers;
	
	/**
	* Код возврата
	*/
	int exitCode;
	
	/**
	* Признак активности демона
	* TRUE - демон работает
	* FALSE - демон остановлен или в процессе остановки
	*/
	bool active;
	
	/**
	* Точка входа в воркер
	*/
	static void * workerEntry(void *pContext);
	
	/**
	* Ожидать поток
	*/
	AsyncStream* waitStream();
	
	/**
	* Возобновить работу с потоком
	*/
	bool resetStream(AsyncStream *stream);
	
protected:
	/**
	* Запустить воркеров
	*/
	void startWorkers();
	
	/**
	* Остановить воркеров
	*/
	void stopWorkers();
	
	/**
	* Обработка системной ошибки
	*/
	void stderror();
public:
	/**
	* Конструктор демона
	* @param maxStreams максимальное число одновременных виртуальных потоков
	*/
	NetDaemon(int maxStreams);
	
	/**
	* Деструктор демона
	*/
	virtual ~NetDaemon();
	
	/**
	* Вернуть число воркеров
	*/
	int getWorkerCount();
	
	/**
	* Установить число воркеров
	*/
	void setWorkerCount(int count);
	
	/**
	* Добавить поток
	*/
	bool addStream(AsyncStream *stream);
	
	/**
	* Удалить поток
	*/
	bool removeStream(AsyncStream *stream);
	
	/**
	* Запустить демона
	*/
	int run();
	
	/**
	* Завершить работу демона
	*/
	void terminate(int code);
	
	/**
	* Обработчик ошибок
	*
	* По умолчанию выводит все ошибки в stderr
	*/
	virtual void onError(const char *message);
};

#endif // NANOSOFT_NETDAEMON_H
