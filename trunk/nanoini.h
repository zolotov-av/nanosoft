#ifndef __NANO_INI_H_
#define __NANO_INI_H_

typedef struct ini_t *ini_p;

/**
* Создать чистый INI-файл
*/
ini_p ini_new();

/**
* Открыть INI-файл
*/
ini_p ini_open(const char *fileName);

/**
* Вернуть значение параметра
* @param ini INI-файл
* @param section название секции
* @param key название параметра
* @param defaultValue значение по умолчанию
* @return значение параметра
*/
const char *ini_get(ini_p ini, const char *section, const char *key, const char *defaultValue);

/**
* Записать значение параметра
* @param ini INI-файл
* @param section название секции
* @param key название параметра
* @param value значение параметра
* @return 1 - в случае удачи и 0 - в случае не удачи
*/
int ini_set(ini_p ini, const char *section, const char *key, const char *value);

/**
* Удалить значение параметра
* @param ini INI-файл
* @param section название секции
* @param key название параметра
* @return 1 - в случае удачи и 0 - в случае не удачи
*/
int ini_remove_option(ini_p ini, const char *section, const char *key);

/**
* Удалить секцию
* @param ini INI-файл
* @param section название секции
* @return 1 - в случае удачи и 0 - в случае не удачи
*/
int ini_remove_section(ini_p ini, const char *section);

/**
* Сохранить INI-файл
*/
int ini_save(ini_p ini, const char *fileName);

/**
* Функция обратного вызова для обхода секций
*/
typedef int (*ini_section_callback)(ini_p ini, const char *section, void *balon);

/**
* Пройтись по всем секциям и для каждой вызывать функцию обратного вызова
*/
int ini_map(ini_p ini, ini_section_callback callback, void *balon);

/**
* Функция обратного вызова для обхода опций
*/
typedef int (*ini_option_callback)(ini_p ini, const char *section, const char *key, const char *value, void *balon);

/**
* Пройтись по всем опциям секции и для каждой вызвать функцию обратного вызова
*/
int ini_section_map(ini_p ini, const char *section, ini_option_callback callback, void *balon);

/**
* Закрыть INI-файл
*/
void ini_close(ini_p ini);

#endif // __NANO_INI_H_
