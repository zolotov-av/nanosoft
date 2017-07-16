#ifndef NANOSOFT_STREAM_H
#define NANOSOFT_STREAM_H

#include <sys/types.h>

/**
 * Абстрактный класс потока данных
 */
class stream
{
protected:
	
	/**
	 * Конструктор копий запрещен
	 */
	stream(const stream &) { }
	
	/**
	 * Оператор присваивания запрещен
	 */
	stream& operator = (const stream &) { return *this; }
	
public:
	
	/**
	 * Конструктор по умолчанию
	 */
	stream() { }
	
	/**
	 * Деструктор
	 */
	virtual ~stream() { }
	
	/**
	 * Прочитать данные из потока
	 */
	virtual size_t read(void *buffer, size_t size) = 0;
	
	/**
	 * Записать данные в поток
	 */
	virtual size_t write(const void *buffer, size_t size) = 0;
	
};

#endif // NANOSOFT_STREAM_H
