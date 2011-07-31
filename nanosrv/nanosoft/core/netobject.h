#ifndef NANOSOFT_NETOBJECT_H
#define NANOSOFT_NETOBJECT_H

#include <stdint.h>
#include <sys/types.h>
#include <nanosoft/core/object.h>

/**
* Базовый класс для всех асинхронных объектов
*/
class NetObject: public Object
{
friend class NetDaemon;
private:
	/**
	* Признак завершения
	*/
	bool terminating;
	
	/**
	* Конструктор копий
	*
	* Не ищите его реализации, его нет и не надо.
	* Просто блокируем конкструктор копий по умолчанию
	*/
	NetObject(const NetObject &);
	
	/**
	* Оператор присваивания
	*
	* Блокируем аналогично конструктору копий
	*/
	NetObject& operator = (const NetObject &);
	
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
	
	/**
	* Сигнал завершения работы
	*
	* Сервер решил закрыть соединение, здесь ещё есть время
	* корректно попрощаться с пиром (peer).
	*/
	virtual void onTerminate() = 0;
public:
	/**
	* Конструктор
	*/
	NetObject();
	
	/**
	* Конструктор
	*/
	NetObject(int afd);
	
	/**
	* Деструктор
	*/
	virtual ~NetObject();
	
	/**
	* Неблокирующее чтение из потока
	*/
	ssize_t read(void *buf, size_t count);
	
	/**
	* Неблокирующая запись в поток
	*/
	ssize_t write(const void *buf, size_t count);
	
	/**
	* Послать сигнал завершения
	*/
	void terminate();
};

#endif // NANOSOFT_NETOBJECT_H
