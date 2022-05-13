#include "defs.h"
#include "utils.h"

#include <json.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <syslog.h>
#include <stdarg.h>
#include <errno.h>

#include <security/pam_ext.h>

pam_handle_t *active_pamh = NULL;

void log_message(int priority, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	if (active_pamh) {
		pam_vsyslog(active_pamh, priority, fmt, args);
	} else {
		openlog("web-login", LOG_CONS | LOG_PID, LOG_AUTHPRIV);
		vsyslog(priority, fmt, args);
		closelog();
	}

	va_end(args);
}

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
	/* note: on MacOS, pam_message.msg is a non-const char*, so we need to copy it */
	char * pam_msg = strdup(text);
	if (pam_msg==NULL)
	{
		return NULL;
	}

	PAM_CONST struct pam_message msg = {
		.msg_style = echocode,
		.msg = pam_msg
	};

	PAM_CONST struct pam_message *msgs = &msg;
	struct pam_response *resp = NULL;
	int retval = converse(pamh, 1, &msgs, &resp);

	char *ret = NULL;
	if (retval != PAM_SUCCESS || resp == NULL || resp->resp == NULL ||
		*resp->resp == '\000')
	{
		log_message(LOG_ERR, "Did not receive input from user");
		if (retval == PAM_SUCCESS && resp && resp->resp)
		{
			ret = resp->resp;
		}
	}
	else
	{
		ret = resp->resp;
	}

	/* Deallocate temporary storage */
	if (resp)
	{
		if (!ret)
		{
			free(resp->resp);
		}
		free(resp);
	}
	free(pam_msg);

	return ret;
}

void conv_info(pam_handle_t *pamh, const char *text)
{
	/* note: on MacOS, pam_message.msg is a non-const char*, so we need to copy it */
	char * pam_msg = strdup(text);
	if (pam_msg==NULL)
	{
		log_message(LOG_ERR, "Failed to print info message: %s", strerror(errno));
		return;
	}

	PAM_CONST struct pam_message msg = {
		.msg_style = PAM_TEXT_INFO,
		.msg = pam_msg
	};

	PAM_CONST struct pam_message *msgs = &msg;
	struct pam_response *resp = NULL;
	const int retval = converse(pamh, 1, &msgs, &resp);

	if (retval != PAM_SUCCESS)
	{
		log_message(LOG_ERR, "Failed to print info message");
	}
	free(pam_msg);
	free(resp);
}
