
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nanoini.h"
#include "nanostr.h"

typedef struct ini_option_t
{
  char *key;
  char *value;
} ini_option_t, *ini_option_p;

typedef struct ini_section_t
{
  char *name;
  int noptions;
  ini_option_p options;
} ini_section_t, *ini_section_p;

typedef struct ini_t
{
  int nsections;
  ini_section_p sections;
} ini_t;

/**
* Создать чистый INI-файл
*/
ini_p ini_new()
{
  return (ini_p) calloc(1, sizeof(ini_t));
}

/**
* Открыть INI-файл
*/
ini_p ini_open(const char *fileName)
{
  FILE *f = fopen(fileName, "r");
  if ( f == 0 ) return 0;
  
  ini_p ini = ini_new();
  
  char buf[4096];
  char section[4096] = "_";
  
  while ( fgets(buf, sizeof(buf), f) )
  {
    trim(buf);
    if ( *buf == '[' )
    {
      strncpy(section, buf+1, strlen(buf)-2);
      trim(section);
    }
    char *p = strchr(buf, '=');
    if ( p )
    {
      *p = 0;
      if ( ! ini_set(ini, section, trim(buf), trim(p + 1)) )
      {
        fclose(f);
        ini_close(ini);
        return 0;
      }
    }
  }
  
  fclose(f);
  
  return ini;
}

/**
* Найти опцию в секции
*/
static ini_option_p ini_lookup_option(ini_section_p section, const char *key)
{
  int i;
  ini_option_p option;
  for(i = 0, option = section->options; i < section->noptions; i++, option++)
  {
    if ( strcmp(option->key, key) == 0 ) return option;
  }
  return 0;
}

/**
* Найти секцию
*/
static ini_section_p ini_lookup_section(ini_p ini, const char *name)
{
  int i;
  ini_section_p section;
  for(i = 0, section = ini->sections; i < ini->nsections; i++, section++)
  {
    if ( strcmp(section->name, name) == 0 ) return section;
  }
  return 0;
}

/**
* Вернуть значение параметра
* @param ini INI-файл
* @param section название секции
* @param key название параметра
* @param defaultValue значение по умолчанию
* @return значение параметра
*/
const char *ini_get(ini_p ini, const char *section, const char *key, const char *defaultValue)
{
  ini_section_p s = ini_lookup_section(ini, section);
  if ( s == 0 ) return defaultValue;
  
  ini_option_p o = ini_lookup_option(s, key);
  if ( s == 0 ) return defaultValue;
  
  return o->value;
}

/**
* Вернуть значение параметра
* @param ini INI-файл
* @param section название секции
* @param key название параметра
* @param defaultValue значение по умолчанию
* @return значение параметра
*/
int ini_geti(ini_p ini, const char *section, const char *key, int defaultValue)
{
  ini_section_p s = ini_lookup_section(ini, section);
  if ( s == 0 ) return defaultValue;
  
  ini_option_p o = ini_lookup_option(s, key);
  if ( s == 0 ) return defaultValue;
  
  return atoi(o->value);
}

/**
* Создать новую опцию в секции INI-файла
*/
static ini_option_p ini_option_new(ini_section_p section, const char *key, const char *value)
{
  ini_option_p options = (ini_option_p) realloc(section->options, (section->noptions+1) * sizeof(ini_option_t));
  if ( options == 0 ) return 0;
  section->options = options;
  options[section->noptions].key = strdup(key);
  options[section->noptions].value = strdup(value);
  section->noptions++;
  return options + (section->noptions - 1);
}

/**
* Создать секцию в INI-файле
*/
static ini_section_p ini_section_new(ini_p ini, const char *name)
{
  ini_section_p sections = (ini_section_p) realloc(ini->sections, (ini->nsections+1) * sizeof(ini_section_t));
  if ( sections == 0 ) return 0;
  ini->sections = sections;
  sections[ini->nsections].name = strdup(name);
  sections[ini->nsections].noptions = 0;
  sections[ini->nsections].options = 0;
  ini->nsections++;
  return sections + (ini->nsections - 1);
}

/**
* Записать значение параметра
* @param ini INI-файл
* @param section название секции
* @param key название параметра
* @param value значение параметра
* @return 1 - в случае удачи и 0 - в случае не удачи
*/
int ini_set(ini_p ini, const char *section, const char *key, const char *value)
{
  ini_section_p s = ini_lookup_section(ini, section);
  
  if ( s == 0 )
  {
    s = ini_section_new(ini, section);
    if ( s == 0 ) return 0;
    
    return ini_option_new(s, key, value) ? 1 : 0;
  }
  
  ini_option_p o = ini_lookup_option(s, key);
  if ( o == 0 )
  {
    return ini_option_new(s, key, value) ? 1 : 0;
  }
  
  free(o->value);
  o->value = strdup(value);
  
  return 1;
}

/**
* Записать значение параметра
* @param ini INI-файл
* @param section название секции
* @param key название параметра
* @param value значение параметра
* @return 1 - в случае удачи и 0 - в случае не удачи
*/
int ini_seti(ini_p ini, const char *section, const char *key, int value)
{
  char buf[20];
  sprintf(buf, "%d", value);
  return ini_set(ini, section, key, buf);
}


/**
* Удалить значение параметра
* @param ini INI-файл
* @param section название секции
* @param key название параметра
* @return 1 - в случае удачи и 0 - в случае не удачи
*/
int ini_remove_option(ini_p ini, const char *section, const char *key)
{
  ini_section_p s = ini_lookup_section(ini, section);
  if ( s == 0 ) return 1;
  
  ini_option_p o = ini_lookup_option(s, key);
  if ( o == 0 ) return 1;
  
  // TODO
  return 0;
}

/**
* Удалить секцию
* @param ini INI-файл
* @param section название секции
* @return 1 - в случае удачи и 0 - в случае не удачи
*/
int ini_remove_section(ini_p ini, const char *section)
{
  ini_section_p s = ini_lookup_section(ini, section);
  if ( s == 0 ) return 1;
  
  // TODO
  return 0;
}

/**
* Сохранить секцию в файл
*/
static void ini_section_save(FILE *f, ini_section_p section)
{
  fprintf(f, "[%s]\n", section->name);
  int i;
  ini_option_p option;
  for(i = 0, option = section->options; i < section->noptions; i++, option++)
  {
    fprintf(f, "%s = %s\n", option->key, option->value);
  }
  fprintf(f, "\n");
}

/**
* Сохранить INI-файл
*/
int ini_save(ini_p ini, const char *fileName)
{
  FILE *f = fopen(fileName, "w");
  if ( f == 0 ) return 0;
  
  int i;
  ini_section_p section;
  for(i = 0, section = ini->sections; i < ini->nsections; i++, section++)
  {
    ini_section_save(f, section);
  }
  
  fclose(f);
  return 1;
}

/**
* Удалить опцию
*/
static ini_option_free(ini_option_p option)
{
  free(option->key);
  free(option->value);
}

/**
* Удалить секцию
*/
static ini_section_free(ini_section_p section)
{
  int i;
  for(i = 0; i < section->noptions; i++)
  {
    ini_option_free(section->options + i);
  }
  free(section->options);
  free(section->name);
}

/**
* Пройтись по всем секциям и для каждой вызывать функцию обратного вызова
*/
int ini_map(ini_p ini, ini_section_callback callback, void *balon)
{
  int i, r;
  for(i = 0; i < ini->nsections; i++)
  {
    if ( r = callback(ini, ini->sections[i].name, balon) ) return r;
  }
  return 0;
}

/**
* Пройтись по всем опциям секции и для каждой вызвать функцию обратного вызова
*/
int ini_section_map(ini_p ini, const char *section, ini_option_callback callback, void *balon)
{
  ini_section_p s = ini_lookup_section(ini, section);
  if ( s == 0 ) return 0;
  
  int i, r;
  ini_option_p option;
  for(i = 0, option = s->options; i < s->noptions; i++, option++)
  {
    if ( r = callback(ini, section, option->key, option->value, balon) ) return r;
  }
  return 0;
}

/**
* Закрыть INI-файл
*/
void ini_close(ini_p ini)
{
  int i;
  for(i = 0; i < ini->nsections; i++)
  {
    ini_section_free(ini->sections + i);
  }
  free(ini->sections);
  free(ini);
}
