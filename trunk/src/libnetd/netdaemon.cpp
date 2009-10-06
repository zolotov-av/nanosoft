
#include <sys/epoll.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

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
	* Число потоков-воркеров
	*/
	int workerCount;
	
	/**
	* Список воркеров
	*/
	pthread_t *workers;
	
	/**
	* Добавить поток в epoll
	*/
	bool addStream(int fd, int events);
	
	/**
	* Ожидать поток
	*/
	int waitStream();
	
	/**
	* Точка входа в воркер
	*/
	static void * workerEntry(void *pDaemon);
	
	/**
	* Запустить воркеров
	*/
	void startWorkers();
	
	/**
	* Остановить воркеров
	*/
	void stopWorkers();
public:
	/**
	* Конструктор демона
	* @param maxStreams максимальное число одновременных потоков
	*/
	NetDaemon(int maxStreams);
	
	/**
	* Деструктор демона
	*/
	~NetDaemon();
	
	/**
	* Вернуть число воркеров
	*/
	int getWorkerCount();
	
	/**
	* Установить число воркеров
	*/
	void setWorkerCount(int count);
	
	/**
	* Запустить демона
	*/
	int run();
};

NetDaemon::NetDaemon(int maxStreams)
{
	epoll = epoll_create(maxStreams);
	
	addStream(STDIN_FILENO, EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP | EPOLLET);
	//addStream(STDOUT_FILENO, EPOLLOUT | EPOLLPRI | EPOLLERR | EPOLLHUP);
	//addStream(STDERR_FILENO, EPOLLOUT | EPOLLPRI | EPOLLERR | EPOLLHUP);
}

NetDaemon::~NetDaemon()
{
	close(epoll);
}

int NetDaemon::getWorkerCount()
{
	return workerCount;
}

void NetDaemon::setWorkerCount(int count)
{
	workerCount = count;
}

bool NetDaemon::addStream(int fd, int events)
{
	struct epoll_event event;
	event.events = events;
	event.data.fd = fd;
	return epoll_ctl(epoll, EPOLL_CTL_ADD, fd, &event) == 0;
}

int NetDaemon::waitStream()
{
	struct epoll_event event;
	int r = epoll_wait(epoll, &event, 1, -1);
	return r > 0 ? event.data.fd : r;
}

void * NetDaemon::workerEntry(void *pDaemon)
{
	NetDaemon *d = static_cast<NetDaemon *>(pDaemon);
	
	int fd = d->waitStream();
	char buf[400];
	while ( int r = read(fd, buf, sizeof(buf)-1) )
	{
		buf[r] = 0;
		printf("%s", buf);
	}
	
	return 0;
}

void NetDaemon::startWorkers()
{
	//printf("todo startWorkers\n");
	workers = new pthread_t[workerCount];
	for(int i = 0; i < workerCount; i++)
	{
		pthread_create(&workers[i], NULL, workerEntry, this);
	}
}

void NetDaemon::stopWorkers()
{
	//printf("todo stopWorkers\n");
	delete [] workers;
}

int NetDaemon::run()
{
	startWorkers();
	workerEntry(this);
	stopWorkers();
	return 0;
}

int main()
{
	NetDaemon daemon(1000);
	daemon.setWorkerCount(2);
	return daemon.run();
}
