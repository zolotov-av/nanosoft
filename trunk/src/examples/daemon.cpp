
#include <errno.h>
#include <string.h>
#include <iostream>
#include <exception>
#include <nanosoft/netdaemon.h>

using namespace std;

class TestStream: public AsyncStream
{
public:
	TestStream(int afd): AsyncStream(afd) { }
	
	/**
	* Событие готовности к чтению
	*
	* Вызывается когда в потоке есть данные,
	* которые можно прочитать без блокирования
	*/
	void onRead()
	{
		std::cerr << "TestStream::onRead()" << std::endl;
		char buf[400];
		int r = read(buf, sizeof(buf)-1);
		if ( r < 0 )
		{
			std::cerr << "read() fault: " << strerror(errno) << std::endl;
			throw std::exception();
		}
		if ( r == 0 )
		{
			std::cerr << "read() fault: end of file" << std::endl;
			throw std::exception();
		}
		std::cout << std::string(buf, r);
	}
	
	/**
	* Событие готовности к записи
	*
	* Вызывается, когда в поток готов принять
	* данные для записи без блокировки
	*/
	void onWrite()
	{
		std::cerr << "not implemented TestStream::onWrite()" << std::endl;
	}
	
	/**
	* Событие ошибки
	*
	* Вызывается в случае возникновения какой-либо ошибки
	*/
	void onError(const char *message)
	{
		std::cerr << message << std::endl;
	}
	
	/**
	* Событие закрытие соединения
	*
	* Вызывается если peer закрыл соединение
	*/
	void onShutdown()
	{
		std::cerr << "[TestStream]: peer shutdown connection" << std::endl;
		throw std::exception();
	}
};

class MyDaemon: public NetDaemon
{
public:
	MyDaemon(): NetDaemon(1000)
	{
		if ( ! addStream(new TestStream(STDIN_FILENO)) )
		{
			onError("addStream fault");
			throw exception();
		}
	}
};

int main()
{
	MyDaemon daemon;
	daemon.setWorkerCount(2);
	return daemon.run();
}
