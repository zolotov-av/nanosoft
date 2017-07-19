
#include <nanosoft/bufferstream.h>
#include <string.h>

/**
* Конструктор
*
* BufferStream использует пул блоков (класс BlocksStream) для буферизации
* данных. Конструктор по умолчанию будет использовать глобальный пул,
* который возвращает функция bp_pool(). Глобальный пул должен быть
* предварительно инициализирован функцией bp_init_pool()
*/
BufferStream::BufferStream()
{
	bp = bp_pool();
	size = 0;
	offset = 0;
	quota = 0;
	first = NULL;
	last = NULL;
}

/**
* Конструктор
*
* BufferStream использует пул блоков (класс BlocksStream) для буферизации
* данных. Пул должен существовать всё время пока существует поток
* 
* @param bp ссылка на пул блоков
*/
BufferStream::BufferStream(BlocksPool *pool): bp(pool)
{
	size = 0;
	offset = 0;
	quota = 0;
	first = NULL;
	last = NULL;
}

/**
* Деструктор
*/
BufferStream::~BufferStream()
{
	clear();
}

/**
* Прочитать данные из потока
*/
size_t BufferStream::read(void *buf, size_t buf_len)
{
	// список освободившихся блоков
	nano_block_t *unused = 0;
	
	// приводим указатель к удобному типу
	uint8_t *data = (uint8_t *)buf;
	
	// счетчик прочитанных байт
	size_t len = 0;
	
	// пока в буфере есть данные
	while ( (buf_len > 0) && (size > 0) )
	{
		// вычислим размер актуальных данных в первом блоке
		size_t rest = BLOCKSPOOL_BLOCK_SIZE - offset;
		if ( rest > size ) rest = size;
		
		// читаем данные
		size_t r = rest;
		if ( r > buf_len ) r = buf_len;
		memcpy(data, first->data + offset, r);
		
		data += r;
		size -= r;
		len += r;
		offset += r;
		buf_len -= r;
		
		// если блок записан полностью,
		if ( r == rest )
		{
			// добавить его в список освободившихся
			nano_block_t *block = first;
			first = block->next;
			block->next = unused;
			unused = block;
			
			// смещение в новом первом блоке начинается с нуля
			offset = 0;
		}
	}
	
	bp->free(unused);
	
	return len;
}

/**
* Записать данные в поток
*
* Данный метод либо примет все данные целиком, либо (если нет места)
* не запишет ни одного байта
*/
size_t BufferStream::write(const void *buf, size_t buf_len)
{
	// прежде чем писать, проверим квоту
	if ( quota != 0 && (size + buf_len) > quota )
	{
		// превышение квоты
		return 0;
	}
	
	// приводим указатель к удобному типу
	const uint8_t *data = (const uint8_t *)buf;
	
	// остаток незаписанных данных
	size_t len = buf_len;
	
	// список новых блоков
	nano_block_t *block;
	
	// в буфер уже что-нибудь записано?
	// примечание: начинать читать код легче с ветки else
	if ( size > 0 )
	{
		// смещение к свободной части последнего блока или 0, если последний
		// блок заполнен полностью ("уже прочитанные данные" + "текущий размер")
		size_t pos = (offset + size) % BLOCKSPOOL_BLOCK_SIZE;
		
		// размер свободной части последнего блока
		// (сколько байт надо дописать в текущий последний блок)
		size_t rest = pos > 0 ? BLOCKSPOOL_BLOCK_SIZE - pos : 0;
		
		// размер недостающей части, которую надо выделить из пула
		size_t need_bytes = len - rest;
		
		// новые данные умещаются в остатке последнего блока?
		if ( len <= rest )
		{
			// если данные умешаются, то записать надо не более чем len
			rest = len;
		}
		else
		{
			// если даннеы не умешаются, то надо выделить новые блоки
			block = bp->allocBySize(need_bytes);
			if ( block == 0 ) return 0;
		}
		
		// если последний блок заполнен не полностью, то дописать в него
		// примечание: если бы данный код стоял выше, то было бы понятнее,
		//   однако у нас есть правило - записать всё или ничего, так что
		//   прежде чем что-либо записывать, нам надо было выделить блоки,
		//   если блоки выделить не удалось, то выходим не записав ни одного
		//   байта. Если блоки удалось выделить, то дальше у нас нет ошибок
		//   которые могли бы нам помешать завершить запись.
		if ( pos > 0 )
		{
			memcpy(last->data + pos, data, rest);
			data += rest;
			size += rest;
			len -= rest;
			if ( len == 0 ) return rest;
		}
		
		// если мы сюда попали, значит текущий последний блок заполнен
		// полностью и у нас еще остались данные для записи, при этом у нас
		// должны быть выделены новые блоки, мы их присоединяем к текущему
		// последнему блоку
		last->next = block;
		//last = block; не нужен, т.к. last обновиться далее
	}
	else // size == 0 (если буфер изначально пуст)
	{
		// если буфер пуст изначально, то мы просто выделяем новые блоки
		// и инициализируем переменные
		block = bp->allocBySize(len);
		if ( block == 0 )
		{
			// нет буфера
			return 0;
		}
		
		first = block;
		offset = 0;
	}
	
	// пока остаток данных больше размера блока, записываем целыми блоками
	while ( len >= BLOCKSPOOL_BLOCK_SIZE )
	{
		memcpy(block->data, data, BLOCKSPOOL_BLOCK_SIZE);
		data += BLOCKSPOOL_BLOCK_SIZE;
		size += BLOCKSPOOL_BLOCK_SIZE;
		len -= BLOCKSPOOL_BLOCK_SIZE;
		last = block;
		block = block->next;
	}
	
	// если что-то осталось записанное, то записываем его в последний блок
	if ( len > 0 )
	{
		memcpy(block->data, data, len);
		size += len;
		last = block;
	}
	
	return buf_len;
}

/**
* Очистить данные хранящиеся в буфере
*/
void BufferStream::clear()
{
	bp->free(first);
	size = 0;
	offset = 0;
	first = NULL;
	last = NULL;
}
