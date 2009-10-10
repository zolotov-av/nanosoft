#ifndef NANOSOFT_ASYNCOBJECT_H
#define NANOSOFT_ASYNCOBJECT_H

#include <stdint.h>
#include <sys/types.h>

/**
* Базовый класс для всех асинхронных объектов
*/
class AsyncObject
{
friend class NetDaemon;
private:
	/**
	* Конструктор копий
	*
	* Не ищите его реализации, его нет и не надо.
	* Просто блокируем конкструктор копий по умолчанию
	*/
	AsyncObject(const AsyncObject &);
	
	/**
	* Оператор присваивания
	*
	* Блокируем аналогично конструктору копий
	*/
	AsyncObject& operator = (const AsyncObject &);
	
protected:
	/**
	* Файловый дескриптор объекта
	*/
	int fd;
	
	/**
	* Обработка системной ошибки
	*/
	void stderror();
	
	/**
	* Вернуть маску ожидаемых событий
	*/
	virtual uint32_t getEventsMask() = 0;
	
	/**
	* Обработчик события
	*/
	virtual void onEvent(uint32_t events) = 0;
	
public:
	/**
	* Конструктор
	*/
	AsyncObject();
	
	/**
	* Конструктор
	*/
	AsyncObject(int afd);
	
	/**
	* Деструктор
	*/
	virtual ~AsyncObject();
	
	/**
	* Событие ошибки
	*
	* Вызывается в случае возникновения какой-либо ошибки
	*/
	virtual void onError(const char *message);
	
};

#endif // NANOSOFT_ASYNCOBJECT_H
