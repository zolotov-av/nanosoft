#include <nanosoft/simplepool.h>
#include <nanosoft/error.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

namespace nanosoft
{
	/**
	* Конструктор простого пула
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
	SimplePool::SimplePool(const char *name, size_t size, size_t stack_len)
	{
		this->name = strdup(name);
		
		offset = data = static_cast<char *>(malloc(size));
		limit = data + size;
		
		top = stack = static_cast<char **>(malloc(stack_len * sizeof(*stack)));
		stackLimit = stack + stack_len;
		
		peakMemory = 0;
		peakDepth = 0;
	}
	
	/**
	* Деструктор простого пула
	*/
	SimplePool::~SimplePool()
	{
		free(name);
		free(data);
		free(stack);
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
	void * SimplePool::alloc(size_t size)
	{
		if ( offset + size > limit ) fatal_error("[SimplePool] pool exeeded");
		
		void *p = offset;
		offset += size;
		ptrdiff_t busy = offset - data;
		if ( busy > peakMemory ) peakMemory = busy;
		
		return p;
	}
	
	/**
	* Открыть новый контекст
	*
	* Данная функция запоминает занятый объем, чтобы потом
	* восстановить его методом leave(). Информация сохраняется
	* во внутреннем стеке. Если стек переполнен, то в stderr
	* выводиться сообщение об ошибке и генерируется исключение.
	*/
	void SimplePool::enter()
	{
		if ( top == stackLimit ) fatal_error("[SimplePool] stack exeeded");
		*top = offset;
		top ++;
		ptrdiff_t depth = top - stack;
		if ( depth > peakDepth ) peakDepth = depth;
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
	void SimplePool::leave()
	{
		if ( top == stack ) fatal_error("[SimplePool] stack is empty");
		top --;
		offset = *top;
	}
	
	/**
	* Вернуть название пула
	* @return название пула
	*/
	const char * SimplePool::getPoolName()
	{
		return name;
	}
	
	/**
	* Вернуть объем свободной памяти
	* @return размер свободной память
	*/
	size_t SimplePool::getFreeSize()
	{
		return static_cast<size_t>(limit - offset);
	}
	
	/**
	* Вернуть размер выделенной памяти
	* @return размер выделенной памяти
	*/
	size_t SimplePool::getBusySize()
	{
		return static_cast<size_t>(offset - data);
	}
	
	/**
	* Вернуть глубину контекста
	*/
	size_t SimplePool::getStackDepth()
	{
		return static_cast<size_t>(top - stack);
	}
	
	/**
	* Вернуть максимальный зафиксированный размер занятой памяти
	*
	* Данная функция может использоваться для статистики и эмпирических
	* оценок потребностей в памяти.
	*
	* @return максимальный зафиксированный размер занятой памяти
	*/
	size_t SimplePool::getPeakMemory()
	{
		return peakMemory;
	}
	
	/**
	* Вернуть максимальную зафиксированную глубину стека
	*
	* Данная функция может использоваться для статистики и эмпирической
	* оценки необходимой глубины стека.
	*
	* @return максимальная зафиксированная глубина стека
	*/
	size_t SimplePool::getPeakDepth()
	{
		return peakDepth;
	}
}
