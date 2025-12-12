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
	vsyslog(priority, fmt, args);
	closelog();

	va_end(args);
}

static int converse(pam_handle_t *pamh, int nargs,
					const struct pam_message **message,
					struct pam_response **response)
{
	struct pam_conv *conv;
	int retval = pam_get_item(pamh, PAM_CONV, (const void *const)&conv);
	if (retval != PAM_SUCCESS)
	{
		return retval;
	}
	return conv->conv(nargs, message, response, conv->appdata_ptr);
}

char *tty_input(pam_handle_t *pamh, const char *text, int echo_code)
{
	/* note: on MacOS, pam_message.msg is a non-const char*, so we need to copy it */
	char * pam_msg = strdup(text);
	if (pam_msg==NULL)
	{
		return NULL;
	}

	const struct pam_message msg = {
		.msg_style = echo_code,
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

bool input_is_safe(const char *input, size_t max_length)
{
	size_t length = strnlen(input, max_length);
	if (input[length] != '\0')
	{
		return false;
	}
	/* Allow strings that are valid usernames according to POSIX.
	 * Valid characters are a-z A-Z 0-9 '.' '_' '-' ; the first character must not be '-'.
	 * See POSIX spec:
	 * https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap03.html#tag_03_437
	 * https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap03.html#tag_03_282
         * Additionally allow ':' so that IPv6 addresses are also considered safe. */
	for (size_t i = 0; i < length; i++)
	{
		/* Don't use isalnum() here because it is locale-dependent,
		 * and don't use isalnum_l() because it is not portable. */
		if (!(     (input[i] >= 'a' && input[i] <= 'z')
			|| (input[i] >= 'A' && input[i] <= 'Z')
			|| (input[i] >= '0' && input[i] <= '9')
			|| (input[i] == '.' || input[i] == '_' || input[i] == ':')
			|| (i > 0 && input[i] == '-')))
		{
			return false;
		}
	}
	return true;
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
