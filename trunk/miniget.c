
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>

#include "nanourl.h"

/**
* Открыть сокет
* @param host хост или IP
* @param port порт
* @return сокет | файловый дескриптор
*/
int ht_open(const char *host, const char *port)
{
  if ( port == 0 ) port = "80";
  
  struct addrinfo hints, *addr;
  
  // first, load up address structs with getaddrinfo():
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  
  int t = getaddrinfo(host, port, &hints, &addr);
  if ( t != 0 )
  {
    fprintf(stderr, "getaddrinfo fault\n");
    return 0;
  }
  
  int s = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
  if ( s == -1 )
  {
    fprintf(stderr, "open socket error\n");
    freeaddrinfo(addr);
    return 1;
  }
  
  t = connect(s, addr->ai_addr, addr->ai_addrlen);
  if ( t == -1 )
  {
    fprintf(stderr, "connect fault\n");
    freeaddrinfo(addr);
    close(s);
    return 0;
  }
  
  freeaddrinfo(addr);
  
  return s;
}

/**
* Закрыть соединение
*/
void ht_close(int s)
{
  shutdown(s, SHUT_RDWR);
  close(s);
}

void strtolower(char *str)
{
  for(; *str; str++) *str = tolower(*str);
}

int main(int argc, char **argv)
{
  if ( argc < 2 )
  {
    printf("miniget <URL>\n");
    return 0;
  }
  
  url_p url = url_parse(argv[1]);
  if ( url == 0 )
  {
    fprintf(stderr, "incorrect url\n");
    return 1;
  }
  
  int s = ht_open(url->host, url->port);
  if ( s == 0 )
  {
    fprintf(stderr, "connection fault\n");
    return 1;
  }
  
  char request[4096];
  sprintf(request, "GET %s HTTP/1.0\r\nhost: %s\r\n\r\n", url->path, url->host);
  // "GET /tests/sessions.php HTTP/1.0\r\nhost: test\r\n\r\n";
  int len = strlen(request);
  int r = write(s, request, len);
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
  while ( p <= body )
  {
    char *end = strchr(p, '\n');
    *end = 0;
    //printf("line: %s\n", p);
    char *colon = strchr(p, ':');
    if ( colon )
    {
      *colon++ = 0;
      strtolower(p);
      //printf("key: %s\nvalue: %s\n\n", p, colon);
    }
    p = end + 1;
  }
  
  body += 4;
  
  char *file = strrchr(url->path, '/');
  if ( file == 0 || file[1] == 0 ) file = "index.html";
  else file ++;
  
  printf("saving to file %s\n", file);
  
  int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if ( fd == 0 )
  {
    fprintf(stderr, "open file fault: %s\n", file);
    
    // завершение работы
    shutdown(s, SHUT_RDWR);
    close(s);
  }
  
  r -= body - buf;
  
  write(fd, body, r);
  
  do
  {
    r = read(s, buf, sizeof(buf));
    write(fd, buf, r);
  } while ( r > 0 );
  
  
  close(fd);
  
  // завершение работы
  shutdown(s, SHUT_RDWR);
  close(s);
  
  return 0;
}
