
#include <nanosoft/filestream.h>

#include <unistd.h>

/**
* Конструктор по умолчанию
*/
FileStream::FileStream(): fd(0)
{
}

/**
* Деструктор
*/
FileStream::~FileStream()
{
	close();
}

/**
* Отрыть файл
*/
bool FileStream::open(const char *fname, int flags)
{
	fd = ::open(fname, flags);
	return fd >= 0;
}

/**
* Отрыть файл
*/
bool FileStream::open(const char *fname, int flags, mode_t mode)
{
	fd = ::open(fname, flags, mode);
	return fd >= 0;
}

/**
* Присоединить файловый дескриптор
*/
void FileStream::bind(int afd)
{
	fd = afd;
}

/**
* Прочитать данные из потока
*/
size_t FileStream::read(void *buffer, size_t size)
{
	return ::read(fd, buffer, size);
}

/**
* Записать данные в поток
*/
size_t FileStream::write(const void *buffer, size_t size)
{
	return ::write(fd, buffer, size);
}

/**
* Закрыть файл
*/
void FileStream::close()
{
	::close(fd);
	fd = 0;
}
