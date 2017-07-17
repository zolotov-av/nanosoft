
#include <time.h>
#include <stdio.h>

/**
* Просто заснуть на N секунд, аналог sleep
*/
int main(int argc, char **argv)
{
  if ( argc < 2 )
  {
    fprintf(stderr, "wait <seconds>\n");
    return 1;
  }
  struct timespec tm;
  tm.tv_sec = atoi(argv[1]);
  tm.tv_nsec = 0;
  nanosleep(&tm, 0);
  return 0;
}
