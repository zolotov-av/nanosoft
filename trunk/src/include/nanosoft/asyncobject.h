#ifndef NANOSOFT_ASYNCOBJECT_H
#define NANOSOFT_ASYNCOBJECT_H

#include <stdint.h>
#include <sys/types.h>
#include <nanosoft/object.h>

/**
* Базовый класс для всех асинхронных объектов
*/
class AsyncObject: public nanosoft::Object
{
friend class NetDaemon;
private:
	/**
	* Файловый дескриптор объекта
	*/
	int fd;
	
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
	AsyncObject(const AsyncObject &);
	
	/**
	* Оператор присваивания
	*
	* Блокируем аналогично конструктору копий
	*/
	AsyncObject& operator = (const AsyncObject &);
	
protected:
	
	/**
	* Обработка системной ошибки
	*/
	void stderror();
	
	/**
	* Установить файловый дескриптор
	*/
	void setFd(int v);
	
	/**
	* Вернуть маску ожидаемых событий
	*/
	virtual uint32_t getEventsMask() = 0;
	
	/**
	* Обработчик события
	*/
	virtual void onEvent(uint32_t events) = 0;
	
	/**
	* Событие ошибки
	*
	* Вызывается в случае возникновения какой-либо ошибки
	*/
	virtual void onError(const char *message);
	
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
	AsyncObject();
	
	/**
	* Конструктор
	*/
	AsyncObject(int afd);
	
	/**
	* Вернуть файловый дескриптор
	*/
	int getFd() const { return fd; }
	
	/**
	* Деструктор
	*/
	virtual ~AsyncObject();
	
	/**
	* Послать сигнал завершения
	*/
	void terminate();
};

#endif // NANOSOFT_ASYNCOBJECT_H
