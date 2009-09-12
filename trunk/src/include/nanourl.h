#ifndef __NANO_URL_H_
#define __NANO_URL_H_

/**
* Структура описывающая фрагменты URL
*/
typedef struct
{
  char *scheme;
  char *host;
  char *port;
  char *user;
  char *pass;
  char *path;
  char *query;
  char *fragment;
} url_t, *url_p;

/**
* Парсинг URL-а
*/
url_p url_parse(const char *url);

/**
* Освободить память
*/
void url_free(url_p url);

#endif // __NANO_URL_H_
