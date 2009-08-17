
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>

void strtolower(char *str)
{
  for(; *str; str++) *str = tolower(*str);
}

int main()
{
  // подлючение: создать сокет
  int s = socket(PF_INET, SOCK_STREAM, 0);
  if ( s == -1 )
  {
    fprintf(stderr, "open socket error\n");
    return 1;
  }
  
  // подключение: коннект
  struct sockaddr_in sa;
  memset(&sa, 0, sizeof(sa));
  sa.sin_family = PF_INET;
  sa.sin_port = htons(80);
  int res = inet_pton(PF_INET, "127.0.0.1", &sa.sin_addr);
  if ( res <= 0 )
  {
    fprintf(stderr, "wrong host IP\n");
    close(s);
    return 1;
  }
  res = connect(s, (struct sockaddr*)(&sa), sizeof(sa));
  if ( res == -1 )
  {
    fprintf(stderr, "connect fault\n");
    close(s);
    return 1;
  }
  
  char *request = "GET / HTTP/1.0\r\nhost: och.localhost\r\n\r\n";
  // "GET /tests/sessions.php HTTP/1.0\r\nhost: test\r\n\r\n";
  int len = strlen(request);
  int r = send(s, request, len, 0);
  if ( r != len )
  {
    fprintf(stderr, "send fault (not all data send)\n");
    return 1;
  }
  
  char buf[4096];
  r = recv(s, buf, sizeof(buf)-1, 0);
  buf[r] = 0;
  //printf("response:\n%s\n\n", buf);
  
  char *body = strstr(buf, "\r\n\r\n");
  if ( body == 0 )
  {
    fprintf(stderr, "headers to large\n");
    return 1;
  }
  body[3] = 0;
  //printf("headers:\n%s\n\n", buf);
  
  char *p = strchr(buf, '\n');
  if ( p == 0 )
  {
    fprintf(stderr, "wrong HTTP response\n");
    return 1;
  }
  *p++ = 0;
  printf("head: %s\n", buf);
  
  // парсим заголовки
  /*while ( p <= body )
  {
    char *end = strchr(p, '\n');
    *end = 0;
    printf("line: %s\n", p);
    char *colon = strchr(p, ':');
    if ( colon )
    {
      *colon++ = 0;
      strtolower(p);
      printf("key: %s\nvalue: %s\n\n", p, colon);
    }
    p = end + 1;
  }*/
  
  
  // завершение работы
  shutdown(s, SHUT_RDWR);
  close(s);
  
  return 0;
}
