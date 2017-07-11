
#include <ffcairo/avc_http.h>

#include <ctype.h>
#include <string>

/**
* Конструктор
*/
AVCHttp::AVCHttp(int afd, AVCEngine *e): AsyncStream(afd), engine(e)
{
	http_state = READ_METHOD;
	line.clear();
	done = false;
}

/**
* Деструктор
*/
AVCHttp::~AVCHttp()
{
	printf("AVCHttp::~AVCHttp()\n");
	close();
}

/**
* Обработчик прочитанных данных
*/
void AVCHttp::onRead(const char *data, size_t len)
{
	const char *p = data;
	const char *lim = data + len;
	
	while ( http_state == READ_METHOD || http_state == READ_HEADERS )
	{
		while ( *data != '\n' && data < lim ) data ++;
		line += std::string(p, data);
		if ( data >= lim ) return;
		data++;
		p = data;
		if ( http_state == READ_METHOD )
		{
			method = line;
			printf("method: %s\n", method.c_str());
			http_state = READ_HEADERS;
			line.clear();
		}
		else
		{
			if ( line == "" || line == "\r" )
			{
				http_state = READ_BODY;
			}
			else
			{
				printf("header: %s\n", line.c_str());
				line.clear();
			}
		}
	}
	
	if ( http_state == READ_BODY )
	{
		printf("read body\n");
		
		write("HTTP/1.0 200 OK\r\n");
		write("Content-type: text/plain\r\n");
		write("\r\n");
		write("Hello world");
		done = true;
	}
}

/**
* Обработчик события опустошения выходного буфера
*
* Вызывается после того как все данные были записаны в файл/сокет
*/
void AVCHttp::onEmpty()
{
	printf("onEmtpy()\n");
	if ( done )
	{
		leaveDaemon();
	}
}

/**
* Пир (peer) закрыл поток.
*
* Мы уже ничего не можем отправить в ответ,
* можем только корректно закрыть соединение с нашей стороны.
*/
void AVCHttp::onPeerDown()
{
	// TODO
	disableObject();
}


void AVCHttp::write(const std::string &s)
{
	put(s.c_str(), s.size());
}
