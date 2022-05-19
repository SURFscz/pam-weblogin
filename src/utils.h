#ifndef UTILS_H
#define UTILS_H

#include "defs.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <json-parser/json.h>

extern json_value *findKey(json_value *value, const char *name);
extern char *getString(json_value *, const char *name);
extern bool getBool(json_value *value, const char *name);
extern char *str_printf(const char *fmt, ...);

#endif
