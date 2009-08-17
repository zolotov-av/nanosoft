
#include <time.h>
#include <sys/time.h>
#include <stdio.h>

int main()
{
	struct timeval tm;
	gettimeofday(&tm, 0);
	printf("time: %d, %d\n", tm.tv_sec, tm.tv_usec);
	double microtime =  tm.tv_sec + ( tm.tv_usec / 1000000.0 );
	printf("microtime: %.6f\n", microtime);
	return 0;
}
