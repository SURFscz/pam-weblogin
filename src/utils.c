#include "defs.h"

#include <stdarg.h>
#include <ctype.h>

#include "tty.h"
#include "utils.h"

json_value *findKey(json_value* value, const char* name) {
	json_value * result = NULL;

	if (value == NULL || name == NULL || name[0]=='\0' ) {
		return NULL;
	}

	char *remaining = NULL, *current = strdup(name);

	if (current) {
		if ((remaining=strchr(current, '.')) != NULL) {
			*remaining++ = '\0';
		}

		for (unsigned int x = 0; x < value->u.object.length; x++) {
			if (!strcmp(value->u.object.values[x].name, current)) {

				result = (remaining ? findKey(value->u.object.values[x].value, remaining) : value->u.object.values[x].value);

				break;
			}
		}

		free(current);
	}

	return result;
}

char *getString(json_value *value, const char *name)
{
	json_value *key = findKey(value, name);
	return key ? strdup(key->u.string.ptr) : NULL;
}

char *getKey(json_value *value, const unsigned int index)
{
	return (index < value->u.object.length) ? strdup(value->u.object.values[index].name) : NULL;
}

char *getValue(json_value *value, const unsigned int index)
{
	return (index < value->u.object.length) ? strdup(value->u.object.values[index].value->u.string.ptr) : NULL;
}

bool getBool(json_value *value, const char *name)
{
	json_value *key = findKey(value, name);
	return key ? key->u.boolean : false;
}

char *str_printf(const char * fmt, ...) {
	char *buffer = NULL;
	va_list args;

	if (fmt==NULL)
		return NULL;

	va_start(args, fmt);
	int rc = vasprintf(&buffer, fmt, args);
	va_end(args);

	if (rc < 0) {
		log_message(LOG_ERR, "Error create string with fmt: %s", fmt);
		return NULL;
	}
	return buffer;
}

char *trim(char *s, const size_t len)
{
	if (s == NULL)
		return NULL;

	if (s[0]=='\0')
		return s;

	for (char *t = s + strnlen(s, len) - 1; isspace(*t) && t>s ; t--)
		*t = '\0';

	while (isspace(*s))
		s++;

	return s;
}
