
#include <time.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

/**
* Вернуть текущее время в секундах
*/
double microtime()
{
  struct timeval tm;
  gettimeofday(&tm, 0);
  return tm.tv_sec + tm.tv_usec / 1000000.0;
}

/**
* Замерять время выполнения программы
*/
int main(int argc, char **argv)
{
  double start = microtime();
  
  int pid = fork();
  if ( pid == -1 ) // ошибка
  {
    fprintf(stderr, "fork fault\n");
    return 2;
  }
  
  if ( pid == 0 ) // потомок
  {
    int r = execve(argv[1], argv + 1, 0);
    if ( r == -1 )
    {
      fprintf(stderr, "exec fault\n");
      return 3;
    }
  }
  
  // ждем завершения
  wait(0);
  
  double time = microtime() - start;
  fprintf(stderr, "time: %.6f seconds\n", time);
  return 0;
}
