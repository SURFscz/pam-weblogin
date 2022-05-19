#include <stdarg.h>

#include "tty.h"
#include "utils.h"

json_value *findKey(json_value* value, const char* name) {
	json_value * result = NULL;

	if (value == NULL) {
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

bool getBool(json_value *value, const char *name)
{
	json_value *key = findKey(value, name);
	return key ? key->u.boolean : false;
}

char *str_printf(const char * fmt, ...) {
	char *buffer = NULL;
	va_list args;
	
	va_start(args, fmt);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
	int rc = vasprintf(&buffer, fmt, args);
#pragma clang diagnostic pop
	va_end(args);

	if (rc == -1) {
		log_message(LOG_ERR, "Error create string with fmt: %s", fmt);
		return NULL;
	}
	return buffer;
}
