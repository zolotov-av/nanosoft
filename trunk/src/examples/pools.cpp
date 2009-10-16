
#include <nanosoft/doublepool.h>
#include <string.h>
#include <stdio.h>

using namespace nanosoft;

DoublePool pool("pool", 16 * 1024 * 1024, 12);

int main()
{
	printf("busy: %d\ndepth: %d\n", pool.getBusySize(), pool.getStackDepth());
	pool.enter();
		pool.alloc(1024);
		pool.enter();
			pool.alloc(100);
			pool.tempAlloc(12);
		pool.leave();
		printf("  busy: %d\n  depth: %d\n", pool.getBusySize(), pool.getStackDepth());
	pool.leave();
	printf(" busy: %d\n depth: %d\n", pool.getBusySize(), pool.getStackDepth());
	printf("peak memory: %d\npeak depth: %d\n", pool.getPeakMemory(), pool.getPeakDepth());
	pool.leave();
	return 0;
}
