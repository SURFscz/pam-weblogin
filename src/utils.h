#ifndef UTILS_H
#define UTILS_H

#include "defs.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#include <json-parser/json.h>

json_value *findKey(json_value *value, const char *name);
char *getString(json_value *, const char *name);
bool getBool(json_value *value, const char *name);
char *str_printf(const char *fmt, ...)
     __attribute__ ((format (printf, 1, 2)));

char *trim(char *s);


#endif
