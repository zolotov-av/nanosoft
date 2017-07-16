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
	printf("Hello world\n");
	printf("FDBUFFER_BLOCK_SIZE = %d\n", FDBUFFER_BLOCK_SIZE);
	
	BlocksPool bp;
	bp.reserve(16);
	printf("\n");
	printf("total = %d\n", bp.getTotalCount());
	printf("free = %d\n", bp.getFreeCount());
	
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
