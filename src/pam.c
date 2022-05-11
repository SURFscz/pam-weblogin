#include "pam.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>

/*
 * log_message logs (debug) messages to syslog/auth.log because sshd discards stdout and stderr
 */
/* TODO: add debug logging */
/* TODO: remove pamh here; if we need to relay the service name, pass it as a string */
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
		/* Something really bad happened. There is no way we can proceed safely. */
		/* TODO:  are we ever using this? */
		exit(1);
	}
}

/* TODO: not really utils; move to own pam file */
/*
 * converse is a generic PAM conversation helper
 */
static int converse(pam_handle_t *pamh, int nargs,
                    PAM_CONST struct pam_message **message,
                    struct pam_response **response)
{
	struct pam_conv *conv;
	int ret = pam_get_item(pamh, PAM_CONV, (void *)&conv);
	if (ret != PAM_SUCCESS)
	{
		return ret;
	}
	return conv->conv(nargs, message, response, conv->appdata_ptr);
}

/* TODO:fix return values!  Needs te return NULL on failure! */
/*
 * conv_read creates a PAM conversation and returns the user response
 */
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
	int ret = converse(pamh, 1, &msgs, &resp);

	char *response = NULL;
	if (ret != PAM_SUCCESS || resp == NULL || resp->resp == NULL ||
		*resp->resp == '\000')
	{
		log_message(LOG_ERR, pamh, "Did not receive input from user");
		if (ret == PAM_SUCCESS && resp && resp->resp)
		{
			response = resp->resp;
		}
	}
	else
	{
		response = resp->resp;
	}

	/* Deallocate temporary storage */
	if (resp)
	{
		if (!response)
		{
			/* TODO: might be NULL */
			free(resp->resp);
		}
		free(resp);
	}
	free(pam_msg);

	return response;
}

/*
 * conv_info specifically creates a PAM conversation
 */

void conv_info(pam_handle_t *pamh, const char *text)
{
	/* note: on MacOS, pam_message.msg is a non-const char*, so we need to copy it */
	char * pam_msg = strdup(text);
	if (pam_msg==NULL)
	{
		log_message(LOG_ERR, pamh, "Failed to print info message: %s", strerror(errno));
		return;
	}

	PAM_CONST struct pam_message msg = {
		.msg_style = PAM_TEXT_INFO,
		.msg = pam_msg
	};

	PAM_CONST struct pam_message *msgs = &msg;
	struct pam_response *resp = NULL;
	const int ret = converse(pamh, 1, &msgs, &resp);

	if (ret != PAM_SUCCESS)
	{
		log_message(LOG_ERR, pamh, "Failed to print info message");
	}
	free(pam_msg);
	free(resp);
}
