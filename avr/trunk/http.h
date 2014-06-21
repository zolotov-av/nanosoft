#ifndef __HTTP_H_
#define __HTTP_H_

/**
* Функция обратного вызова принимающая аргументы
* Её нужно определить самостоятельно
*/
void on_parse_param(const char *pname, const char *pvalue);

/**
* Парсер запроса HTTP
*/
void parse_request(const char *buf, int len);

#endif // __HTTP_H_
