
#include <pwd.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char **argv)
{
  if ( argc < 2 )
  {
    fprintf(stderr, "nsu <user> [ <exec> ... ]\n");
    return 1;
  }
  
  struct passwd *pw = getpwnam(argv[1]);
  if ( pw )
  {
    if ( setuid(pw->pw_uid) != 0 ) fprintf(stderr, "setuid fault\n");
    if ( setgid(pw->pw_gid) != 0 ) fprintf(stderr, "setgid fault\n");
  }
  else fprintf(stderr, "user not found: %s\n", argv[1]);
  
  char *prog = argc < 3 ? "/bin/sh" : argv[2];
  printf("execute: %s\n", prog);
  
  execve(prog, argv + (argc < 3 ? 1 : 2), 0);
  fprintf(stderr, "execve fault\n");
  return 1;
}
