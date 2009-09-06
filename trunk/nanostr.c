
#include "nanostr.h"
#include <string.h>
#include <ctype.h>

void strtolower(char *str)
{
  for(; *str; str++) *str = tolower(*str);
}

char * ltrim(char *s)
{
  char *p = s;
  while ( isspace(*p) ) p++;
  return strcpy(s, p);
}

char * rtrim(char *s)
{
  char *p = s, *l = strchr(s, 0);
  while ( 1 )
  {
    while ( *p && !isspace(*p) ) p++;
    if ( *p == 0 )
    {
      *l = 0;
      return s;
    }
    l = p;
    while ( isspace(*p) ) p++;
  }
}

char * trim(char *s)
{
  return rtrim(ltrim(s));
}

