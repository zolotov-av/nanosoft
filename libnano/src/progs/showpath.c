
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
* Запустить N процессов
*/
int main(int argc, char **argv)
{
  char *path = strdup(getenv("PATH"));
  char *save = path;
  
  while ( *path )
  {
    char *p = path;
    while ( *p && *p != ':' ) *p++;
    if ( *p == 0 )
    {
      printf("%s\n", path);
      break;
    }
    *p = 0;
    printf("%s\n", path);
    path = p + 1;
  }
  
  free(save);
  
  return 0;
}
