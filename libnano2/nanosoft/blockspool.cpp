
#include <nanosoft/blockspool.h>

#include <string.h>
#include <stdio.h>

/**
* Конструктор
*/
BlocksPool::BlocksPool()
{
	total_count = 0;
	free_count = 0;
	pool_size = 0;
	stack = NULL;
	pools = NULL;
}

/**
* Деструктор
*
* Высвобождает все блоки, в том числе те, что выданы клиентам
*/
BlocksPool::~BlocksPool()
{
	clear();
}

static void assert_align(void *p)
{
	uintptr_t addr = (uintptr_t)p;
	if ( addr % BLOCKSPOOL_BLOCK_SIZE )
	{
		printf("block not aligned\n");
		exit(-1);
	}
}

/**
* Зарезервировать блоки
*
* Запрашивает у системы новые блоки и добавляет их в пул
*/
int BlocksPool::reserve(size_t count)
{
	size_t info_blocks = (count * sizeof(nano_block_t) + BLOCKSPOOL_BLOCK_SIZE - 1) / BLOCKSPOOL_BLOCK_SIZE;
	size_t info_size = info_blocks * BLOCKSPOOL_BLOCK_SIZE;
	size_t data_size = count * BLOCKSPOOL_BLOCK_SIZE;
	
	void *p;
	int ret = posix_memalign(&p, BLOCKSPOOL_BLOCK_SIZE, info_size + data_size);
	if ( ret != 0 ) return -1;
	
	pool_t *pool = new pool_t;
	pool->data = p;
	pool->next = pools;
	pools = pool;
	
	nano_block_t *info = (nano_block_t*)p;
	uint8_t *data = (uint8_t*)p + info_size;
	
	for(size_t i = 0; i < count; i++)
	{
		info->data = data;
		info->next = stack;
		assert_align(info->data);
		//memset(info->data, 0, BLOCKSPOOL_BLOCK_SIZE);
		stack = info;
		info++;
		data += BLOCKSPOOL_BLOCK_SIZE;
	}
	
	total_count += count;
	free_count += count;
	pool_size += data_size;
	
	return -1;
}

/**
* Зарезервировать блоки
*
* Запрашивает у системы новые блоки и добавляет их в пул
*/
int BlocksPool::reserveBySize(size_t size)
{
	// размер в блоках
	size_t count = (size + BLOCKSPOOL_BLOCK_SIZE - 1) / BLOCKSPOOL_BLOCK_SIZE;
	
	return reserve(count);
}

/**
* Очистить пул
*
* Высвобождает все блоки, в том числе те, что выданы клиентам
*/
void BlocksPool::clear()
{
	while ( pools )
	{
		pool_t *pool = pools;
		pools = pools->next;
		::free(pool->data);
		delete pool;
	}
	
	total_count = 0;
	free_count = 0;
	pool_size = 0;
	stack = 0;
}

/**
* Выделить цепочку блоков
*
* @param size требуемый размер в байтах
* @return список блоков или NULL если невозможно выделить запрощенный размер
*/
nano_block_t* BlocksPool::allocBySize(size_t size)
{
	// размер в блоках
	size_t count = (size + BLOCKSPOOL_BLOCK_SIZE - 1) / BLOCKSPOOL_BLOCK_SIZE;
	
	return allocByBlocks(count);
}

/**
* Выделить цепочку блоков
*
* @param count требуемый размер в блоках
* @return список блоков или NULL если невозможно выделить запрощенный размер
*/
nano_block_t* BlocksPool::allocByBlocks(size_t count)
{
	if ( count == 0 ) return 0;
	if ( count > free_count ) return 0;
	
	nano_block_t *block = stack;
	if ( count > 1 )
	{
		size_t lim = count - 1;
		for(size_t i = 0; i < lim; i++)
		{
			stack = stack->next;
		}
	}
	nano_block_t *last = stack;
	stack = last->next;
	last->next = 0;
	free_count -= count;
	
	return block;
}

/**
* Освободить цепочку блоков
*
* NOTE Этот метод не проверяет принадлежат ли ему эти блоки или другому
* пулу, просто добавит их в свой пул. Надо быть аккуратным чтобы
* возвращать блоки в тот пул из которого они были взяты
*
* @param list цепочка блоков
*/
void BlocksPool::free(nano_block_t *list)
{
	if ( list == 0 ) return;
	nano_block_t *last = list;
	size_t count = 1;
	while ( last->next )
	{
		count++;
		last = last->next;
	}
	last->next = stack;
	stack = list;
	free_count += count;
}

/**
* Глобальный пул
*/
static BlocksPool *global_pool;

/**
* Вернуть глобальный пул
*
* Для инициализации используйте bp_init_pool()
*/
BlocksPool* bp_pool()
{
	return global_pool;
}

/**
* Вернуть глобальный пул
*
* Если пул не был инициализирован, то он будет автоматически инициализирован,
* если пул имеет недостаточный размер, то он будет дополнен, если дополнить
* не удается, то функция вернет NULL.
*/
BlocksPool* bp_pool(size_t min_size)
{
	if ( bp_init_pool(min_size) )
	{
		return global_pool;
	}
	
	return NULL;
}

/**
* Инициализация глобального пула
*
* Функция bp_init_pool() является реэнтерабельной. Если пул еще не был
* инициализирован, то он будет инициализирован с размером не менее указанного.
* Если пул уже был инициализирован, то убедится размер соответствует указанному
* минимальному размеру, если нет, то дополнит его до требуемого размера.
*/
bool bp_init_pool(size_t min_size)
{
	if ( ! global_pool )
	{
		global_pool = new BlocksPool();
	}
	
	if ( global_pool->getTotalCount() < min_size )
	{
		return global_pool->reserve(min_size - global_pool->getTotalCount());
	}
	
	return true;
}
