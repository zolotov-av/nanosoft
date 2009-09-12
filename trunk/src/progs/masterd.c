
#include <sys/types.h>
#include <limits.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>

#include "nanoini.h"

int run_worker(ini_p ini, const char *section, pid_t uid, gid_t gid)
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
    if ( uid ) if ( setuid(uid) != 0 ) fprintf(stderr, "setuid(%d) fault\n", uid);
    if ( gid ) if ( setgid(gid) != 0 ) fprintf(stderr, "setgid(%d) fault\n", gid);
    
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

int start_worker(ini_p ini, const char *section, void *balon)
{
  char path[PATH_MAX];
  realpath(ini_get(ini, section, "exec", 0), path);
  printf("start worker: %s (%s)\n", section, path);
  
  pid_t uid = 0;
  gid_t gid = 0;
  
  const char *user = ini_get(ini, section, "user", 0);
  if ( user )
  {
    struct passwd *pw = getpwnam(user);
    if ( pw )
    {
      uid = pw->pw_uid;
      gid = pw->pw_gid;
    }
    else fprintf(stderr, "user not found: %s\n", user);
  }
  
  const char *groupName = ini_get(ini, section, "group", 0);
  if ( groupName )
  {
    struct group *gr = getgrnam(groupName);
    if ( gr ) gid = gr->gr_gid;
    else fprintf(stderr, "group not found: %s\n", groupName);
  }
  
  printf("user: %s (%d)\ngroup: %s (%d)\n", user, uid, groupName, gid);
  
  ini_seti(ini, section, "uid", uid);
  ini_seti(ini, section, "gid", gid);
  
  return run_worker(ini, section, uid, gid);
}

int restart_worker(ini_p ini, const char *section, void *balon)
{
  if ( ini_geti(ini, section, "pid", 0) == *(int *)balon )
  {
    int uid = ini_geti(ini, section, "uid", 0);
    int gid = ini_geti(ini, section, "gid", 0);
    return run_worker(ini, section, uid, gid);
  }
  return 0;
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
