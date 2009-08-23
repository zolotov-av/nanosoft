
#include "nanoini.h"
#include <stdio.h>
#include <limits.h>

int start_worker(ini_p ini, const char *section, void *balon)
{
  char path[PATH_MAX];
  realpath(ini_get(ini, section, "exec", 0), path);
  printf("start worker: %s (%s)\n", section, path);
  
  int pid = fork();
  if ( pid == -1 ) // ошибка
  {
    fprintf(stderr, "fork fault\n");
    return 0;
  }
  if ( pid == 0 ) // потомок
  {
    const char *args[3] = {
      path,
      ini_get(ini, section, "config", 0),
      0};
    int r = execve(path, args, 0);
    if ( r == -1 )
    {
      fprintf(stderr, "exec fault\n");
      return 0;
    }
  }
  
  ini_seti(ini, section, "pid", pid);
  printf("pid: %d\n", pid);
  return 0;
}

int restart_worker(ini_p ini, const char *section, void *balon)
{
  if ( ini_geti(ini, section, "pid", 0) == *(int *)balon )
  {
    start_worker(ini, section, 0);
  }
}

int main(int argc, char **argv)
{
  const char *configFile = argc > 1 ? argv[1] : "/etc/masterd.ini";
  
  ini_p ini = ini_open(configFile);
  if ( ini == 0 )
  {
    fprintf(stderr, "read config fault: %s\n", configFile);
    return 1;
  }
  
  ini_map(ini, start_worker, 0);
  
  while ( 1 )
  {
    int pid = wait(0);
    if ( pid > 0 )
    {
      printf("[%d] exited\n", pid);
      ini_map(ini, restart_worker, &pid);
    }
  }
  
  return 0;
}
