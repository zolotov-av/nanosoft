
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

/**
* Запустить N процессов
*/
int main(int argc, char **argv)
{
  // проверяем число аргументов
  if ( argc < 3 )
  {
    fprintf(stderr, "forker count /path/to/exe [ args ]\n");
    return 1;
  }
  
  // число запускаемых процессов
  int count = atoi(argv[1]);
  
  // запустить N процессов
  int i;
  for(i = 0; i < count; i++)
  {
    int pid = fork();
    if ( pid == -1 ) // ошибка
    {
      fprintf(stderr, "fork fault\n");
      return 2;
    }
    if ( pid == 0 ) // потомок
    {
      int r = execve(argv[2], argv + 2, 0);
      if ( r == -1 )
      {
        fprintf(stderr, "exec fault\n");
        return 3;
      }
    }
  }
  
  // ожидаем завершения всех запущеных программ
  for(i = 0; i < count; i++)
  {
    wait(0);
  }
  
  return 0;
}
