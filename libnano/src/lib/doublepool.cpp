#include <sys/types.h>
#include <nanosoft/simplepool.h>
#include <nanosoft/doublepool.h>

namespace nanosoft
{
	/**
	* Обменять пулы местами
	*/
	void DoublePool::swap()
	{
		SimplePool *p = active;
		active = temp;
		temp = p;
	}
	
	/**
	* Конструктор двойного пула
	*
	* Параметры size и stack_len определяют размер пула
	* и размер внутреннего стека контекстов. Для нормальной
	* работы пула предполагается, что разработчик или
	* администратор приложения в состоянии оценить требуемые
	* размеры. В случае превышения данных характеристик
	* соответствующие функции выводят в stderr сообщение об
	* ошибке и генерируют исключение.
	*
	* @param name название пула для вывода диагностических сообщений
	* @param size размер пула памяти
	* @param stack_len размер стека контекстов (максимальное число
	*   вложенных контекстов)
	*/
	DoublePool::DoublePool(const char *name, size_t size, size_t stack_len)
	{
		active = new SimplePool(name, size, stack_len / 2);
		temp = new SimplePool(name, size, stack_len / 2);
	}
	
	/**
	* Деструктор двойного пула
	*/
	DoublePool::~DoublePool()
	{
		delete active;
		delete temp;
	}
	
	/**
	* Выделить память из пула
	*
	* Если память выделить не удается, то данная функция выводит
	* сообщение об ошибке в stderr и генерирует исключение.
	*
	* @param size требуемый размер блока
	* @return указатель на выделеный блок
	*/
	void * DoublePool::alloc(size_t size)
	{
		return active->alloc(size);
	}
	
	/**
	* Выделить память из пула временных объектов
	*
	* Если память выделить не удается, то данная функция выводит
	* сообщение об ошибке в stderr и генерирует исключение.
	*
	* @param size требуемый размер блока
	* @return указатель на выделеный блок
	*/
	void * DoublePool::tempAlloc(size_t size)
	{
		return temp->alloc(size);
	}
	
	/**
	* Открыть новый контекст
	*
	* Данная функция запоминает занятый объем, чтобы потом
	* восстановить его методом leave(). Информация сохраняется
	* во внутреннем стеке. Если стек переполнен, то в stderr
	* выводиться сообщение об ошибке и генерируется исключение.
	*/
	void DoublePool::enter()
	{
		swap();
		temp->enter();
	}
	
	/**
	* Закрыть контекст
	*
	* Данная функция закрывает контекст и "освобождает"
	* всю память которая была запрошена внутри этого
	* контекста.
	*
	* Если внутренный стек контекстов пуст, то в stderr
	* выводиться сообщение об ошибке и генерируется исключение.
	*/
	void DoublePool::leave()
	{
		temp->leave();
		swap();
	}
	
	/**
	* Вернуть название пула
	* @return название пула
	*/
	const char * DoublePool::getPoolName()
	{
		return active->getPoolName();
	}
	
	/**
	* Вернуть объем свободной памяти
	* @return размер свободной память
	*/
	size_t DoublePool::getFreeSize()
	{
		return active->getFreeSize() + temp->getFreeSize();
	}
	
	/**
	* Вернуть размер выделенной памяти
	* @return размер выделенной памяти
	*/
	size_t DoublePool::getBusySize()
	{
		return active->getBusySize() + temp->getBusySize();
	}
	
	/**
	* Вернуть глубину контекста
	*/
	size_t DoublePool::getStackDepth()
	{
		return active->getStackDepth() + temp->getStackDepth();
	}
	
	/**
	* Вернуть максимальный зафиксированный размер занятой памяти
	*
	* Данная функция может использоваться для статистики и эмпирических
	* оценок потребностей в памяти.
	*
	* @return максимальный зафиксированный размер занятой памяти
	*/
	size_t DoublePool::getPeakMemory()
	{
		// TODO нужно что-то более правильное...
		return active->getPeakMemory() + temp->getPeakMemory();
	}
	
	/**
	* Вернуть максимальную зафиксированную глубину стека
	*
	* Данная функция может использоваться для статистики и эмпирической
	* оценки необходимой глубины стека.
	*
	* @return максимальная зафиксированная глубина стека
	*/
	size_t DoublePool::getPeakDepth()
	{
		// TODO нужно что-то более правильное...
		return active->getPeakDepth() + temp->getPeakDepth();
	}
}
