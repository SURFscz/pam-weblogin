#ifndef UTILS_H
#define UTILS_H

#include "defs.h"

#include <stdbool.h>
#include <json.h>

#ifndef asprintf
extern int asprintf(char **restrict strp, const char *restrict fmt, ...);
#endif

/* JSON helpers */
json_value *findKey(json_value *value, const char *name);
char *getString(json_value *, const char *name);
bool getBool(json_value *value, const char *name);

#endif
