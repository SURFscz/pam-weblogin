#include "defs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <stdarg.h>
#include <errno.h>

#include "tty.h"

void log_message(int priority, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	openlog("web-login", LOG_CONS | LOG_PID, LOG_AUTHPRIV);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
	vsyslog(priority, fmt, args);
#pragma clang diagnostic pop
	closelog();

	va_end(args);
}

static int converse(pam_handle_t *pamh, int nargs,
					const struct pam_message **message,
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

char *tty_input(pam_handle_t *pamh, const char *text, int echocode)
{
	/* note: on MacOS, pam_message.msg is a non-const char*, so we need to copy it */
	char * pam_msg = strdup(text);
	if (pam_msg==NULL)
	{
		return NULL;
	}

	const struct pam_message msg = {
		.msg_style = echocode,
		.msg = pam_msg
	};

	const struct pam_message *msgs = &msg;
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

void tty_output(pam_handle_t *pamh, const char *text)
{
	/* note: on MacOS, pam_message.msg is a non-const char*, so we need to copy it */
	char * pam_msg = strdup(text);
	if (pam_msg==NULL)
	{
		log_message(LOG_ERR, "Failed to print info message: %s", strerror(errno));
		return;
	}

	const struct pam_message msg = {
		.msg_style = PAM_TEXT_INFO,
		.msg = pam_msg
	};

	const struct pam_message *msgs = &msg;
	struct pam_response *resp = NULL;
	const int retval = converse(pamh, 1, &msgs, &resp);

	if (retval != PAM_SUCCESS)
	{
		log_message(LOG_ERR, "Failed to print info message");
	}
	free(pam_msg);
	free(resp);
}
