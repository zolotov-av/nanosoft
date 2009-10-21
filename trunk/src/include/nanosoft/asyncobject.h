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
	* Номер воркера обрабатывающий в данный момент объект
	*/
	int workerId;
	
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
	
	/**
	* Сигнал завершения работы
	*
	* Объект должен закрыть файловый дескриптор
	* и освободить все занимаемые ресурсы
	*/
	virtual void onTerminate() = 0;
	
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
	* Вернуть ID воркера обрабатывающий объект
	*/
	int getWorkerId() { return workerId; }
	
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
