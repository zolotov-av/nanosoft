#ifndef NANOSOFT_FILESTREAM_H
#define NANOSOFT_FILESTREAM_H

#include <nanosoft/stream.h>

#include <fcntl.h>

/**
 * Файловый поток
 */
class FileStream: public stream
{
protected:
	
	/**
	 * Файловый дескриптор
	 */
	int fd;
	
public:
	
	enum {
		ro = O_RDONLY,
		rw = O_RDWR
	};
	
	/**
	 * Конструктор по умолчанию
	 */
	FileStream();
	
	/**
	 * Деструктор
	 */
	~FileStream();
	
	/**
	 * Открыть файл
	 */
	bool open(const char *fname, int flags);
	
	/**
	 * Отрыть файл
	 */
	bool open(const char *fname, int flags, mode_t mode);
	
	/**
	 * Присоединить файловый дескриптор
	 */
	void bind(int afd);
	
	/**
	 * Прочитать данные из потока
	 */
	virtual size_t read(void *buffer, size_t size);
	
	/**
	 * Записать данные в поток
	 */
	virtual size_t write(const void *buffer, size_t size);
	
	/**
	 * Закрыть файл
	 */
	void close();
	
};

#endif // NANOSOFT_FILESTREAM_H
