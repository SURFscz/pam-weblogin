#include "utils.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <json.h>

/* TODO: NB these are the dangerous functions. These take user-provided input and return them to the system.
 *       Do extra sanitation and validation here.  We can't just rely on libjson-parsser
 *       Might make sense to isolate them in a separate file.
 */

/*
 * findKey tries to find the (first level) named key in a json dict
 */
json_value *findKey(json_value *value, const char *name)
{
	if (value == NULL)
	{
		return NULL;
	}
	for (unsigned x = 0; x < value->u.object.length; x++)
	{
		// log_message(LOG_INFO, pamh, "object[%d].name = %s\n", x, value->u.object.values[x].name);
		/* TODO: are we ABSOLUTELY SURE that the json parser will NEVER return unterminated string or some other weirdness?!
		         Note that the json can contain non-validated user input! (username, email, etc)
		 */
		if (!strcmp(value->u.object.values[x].name, name))
		{
			return value->u.object.values[x].value;
		}
	}
	return NULL;
}

/*
 * getString tries to find the string value of the (first level) named key in a json dict
 * If no key can be found, return NULL as default
 */
char *getString(json_value *value, const char *name)
{
	json_value *key = findKey(value, name);
	if (key == NULL)
	{
		return NULL;
	}
	/* TODO: wouldn't it be safer to return a strdup? */
	return key->u.string.ptr;
}

/*
 * getBool tries to find the boolean value of the (first level) named key in a json dict
 * If no key van be found, return false as default
 */
bool getBool(json_value *value, const char *name)
{
	json_value *key = findKey(value, name);
	if (key == NULL)
	{
		return false;
	}
	return key->u.boolean;
}

/* TODO: end of danger zone */

