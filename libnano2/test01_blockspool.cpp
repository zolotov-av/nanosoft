/****************************************************************************

Тест №01: тест пула блоков

****************************************************************************/

#include <stdio.h>

#include <nanosoft/blockspool.h>

int count_blocks(nano_block_t *p)
{
	int count = 0;
	while ( p )
	{
		count ++;
		p = p->next;
	}
	return count;
}

int main()
{
	printf("test class BlocksPool\n");
	
	BlocksPool bp;
	bp.reserve(16);
	printf("\n");
	printf("total = %d\n", bp.getTotalCount());
	printf("free = %d\n", bp.getFreeCount());
	printf("pool_size = %d\n", (int)bp.getPoolSize());
	
	nano_block_t *blocks = bp.allocBySize(4000);
	printf("\n");
	printf("allocated blocks: %d\n", count_blocks(blocks));
	printf("total = %d\n", bp.getTotalCount());
	printf("free = %d\n", bp.getFreeCount());
	
	bp.reserve(16);
	printf("\n");
	printf("total = %d\n", bp.getTotalCount());
	printf("free = %d\n", bp.getFreeCount());
	
	bp.free(blocks);
	printf("\n");
	printf("total = %d\n", bp.getTotalCount());
	printf("free = %d\n", bp.getFreeCount());
	
	bp.clear();
	printf("\n");
	printf("total = %d\n", bp.getTotalCount());
	printf("free = %d\n", bp.getFreeCount());
}
