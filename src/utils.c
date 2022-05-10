#include "defs.h"
#include "utils.h"

#include <json.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <syslog.h>
#include <stdarg.h>

extern int asprintf(char **restrict strp, const char *restrict fmt, ...);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
void log_message(int priority, pam_handle_t *pamh, const char *format, ...)
{
	char *service = NULL;
	if (pamh)
		pam_get_item(pamh, PAM_SERVICE, (void *)&service);
	if (!service)
		service = "";

	char logname[80];
	snprintf(logname, sizeof(logname), "websso (%s)", service);

	va_list args;
	va_start(args, format);
	openlog(logname, LOG_CONS | LOG_PID, LOG_AUTHPRIV);
	vsyslog(priority, format, args);
	closelog();
	va_end(args);

	if (priority == LOG_EMERG)
	{
		// Something really bad happened. There is no way we can proceed safely.
		exit(1);
	}
}
#pragma clang diagnostic pop

json_value *findKey(json_value *value, const char *name)
{
	if (value == NULL)
	{
		return NULL;
	}
	for (unsigned x = 0; x < value->u.object.length; x++)
	{
		// log_message(LOG_INFO, pamh, "object[%d].name = %s\n", x, value->u.object.values[x].name);
		if (!strcmp(value->u.object.values[x].name, name))
		{
			return value->u.object.values[x].value;
		}
	}
	return NULL;
}

char *getString(json_value *value, const char *name)
{
	json_value *key = findKey(value, name);
	if (key == NULL)
	{
		return NULL;
	}
	return key->u.string.ptr;
}

bool getBool(json_value *value, const char *name)
{
	json_value *key = findKey(value, name);
	if (key == NULL)
	{
		return false;
	}
	return key->u.boolean;
}

static int converse(pam_handle_t *pamh, int nargs,
					PAM_CONST struct pam_message **message,
					struct pam_response **response)
{
	struct pam_conv *conv;
	int retval = pam_get_item(pamh, PAM_CONV, (void *)&conv);
	if (retval != PAM_SUCCESS)
	{
		return retval;
	}
	return conv->conv(nargs, message, response, conv->appdata_ptr);
}

char *conv_read(pam_handle_t *pamh, const char *text, int echocode)
{
	PAM_CONST struct pam_message msg = {
		.msg_style = echocode,
		.msg = text};

	PAM_CONST struct pam_message *msgs = &msg;
	struct pam_response *resp = NULL;
	int retval = converse(pamh, 1, &msgs, &resp);

	char *ret = NULL;
	if (retval != PAM_SUCCESS || resp == NULL || resp->resp == NULL ||
		*resp->resp == '\000')
	{
		log_message(LOG_ERR, pamh, "Did not receive input from user");
		if (retval == PAM_SUCCESS && resp && resp->resp)
		{
			ret = resp->resp;
		}
	}
	else
	{
		ret = resp->resp;
	}

	// Deallocate temporary storage
	if (resp)
	{
		if (!ret)
		{
			free(resp->resp);
		}
		free(resp);
	}
	return ret;
}

void conv_info(pam_handle_t *pamh, const char *text)
{
	PAM_CONST struct pam_message msg = {
		.msg_style = PAM_TEXT_INFO,
		.msg = text};

	PAM_CONST struct pam_message *msgs = &msg;
	struct pam_response *resp = NULL;
	const int retval = converse(pamh, 1, &msgs, &resp);

	if (retval != PAM_SUCCESS)
	{
		log_message(LOG_ERR, pamh, "Failed to print info message");
	}
	free(resp);
}
