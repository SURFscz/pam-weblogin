#include "defs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

#include <security/pam_appl.h>
#include <security/pam_modules.h>

#include <json-parser/json.h>

#include "config.h"
#include "http.h"
#include "utils.h"
#include "tty.h"

#include "pam_weblogin.h"

PAM_EXTERN int pam_sm_setcred(UNUSED pam_handle_t *pamh, UNUSED int flags, UNUSED int argc, UNUSED const char *argv[])
{
	return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_acct_mgmt(UNUSED pam_handle_t *pamh, UNUSED int flags, UNUSED int argc, UNUSED const char *argv[])
{
	return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, UNUSED int flags, int argc, const char *argv[])
{
	char *session_id = NULL;
	char *challenge = NULL;
	char *info = NULL;
	char *authorization = NULL;
	bool cached = false;
	bool error = false;
	char *message = NULL;
	int pam_result = PAM_AUTH_ERR;

	log_message(LOG_INFO, "Start of pam_weblogin");

	/* Read username */
	const char *username;
	if (pam_get_user(pamh, &username, PROMPT_USERNAME) != PAM_SUCCESS)
	{
		log_message(LOG_ERR, "Error getting user");
		return PAM_SYSTEM_ERR;
	}

	/* Read configuration file */
	Config *cfg = NULL;
	if (!(cfg = getConfig((argc > 0) ? argv[0] : DEFAULT_CONF_FILE)))
	{
		tty_output(pamh, "Error reading conf");
		return PAM_SYSTEM_ERR;
	}
/*
	log_message(LOG_INFO, "cfg->url: '%s'\n", cfg->url);
	log_message(LOG_INFO, "cfg->token: '%s'\n", cfg->token);
	log_message(LOG_INFO, "cfg->attribute: '%s'\n", cfg->attribute);
	log_message(LOG_INFO, "cfg->cache_duration: '%d'\n", cfg->cache_duration);
	log_message(LOG_INFO, "cfg->retries: '%d'\n", cfg->retries);
*/

	authorization = str_printf("Authorization: %s", cfg->token);

	/* Prepare full start url... */
	char *url = NULL;
	url = str_printf("%s/%s", cfg->url, API_START_PATH);

	/* Prepare start input data... */
	char *data = NULL;
	data = str_printf("{\"user_id\":\"%s\",\"attribute\":\"%s\",\"cache_duration\":\"%d\",\"GIT_COMMIT\":\"%s\",\"JSONPARSER_GIT_COMMIT\":\"%s\"}",
			 username, cfg->attribute, cfg->cache_duration, TOSTR(GIT_COMMIT), TOSTR(JSONPARSER_GIT_COMMIT));

	/* Request auth session_id/challenge */
	json_char *challenge_response = (json_char *) API(
		url,
		"POST",
		(char *[]){ "Content-Type: application/json", authorization, NULL},
		data
	);
	free(url);
	free(data);

	/* Something went wrong on the server */
	if (challenge_response == NULL)
	{
		log_message(LOG_ERR, SERVER_ERROR);
		tty_output(pamh, SERVER_ERROR);
		pam_result = PAM_SYSTEM_ERR;
		goto finalize;
	}

	/* Parse response */
	json_value *challenge_json = json_parse(challenge_response, strnlen(challenge_response, BUFSIZ));
	free(challenge_response);

	error = getBool(challenge_json, "error");

	/* See if the response contains an error */
	if (error)
	{
		/* See if the response contains a message */
		message = getString(challenge_json, "message");
		if (message != NULL)
		{
			log_message(LOG_INFO, "%s", message);
			tty_output(pamh, message);
		} else
		{
			log_message(LOG_INFO, "%s", SERVER_NO_MESSAGE);
			tty_output(pamh, SERVER_NO_MESSAGE);
		}
		json_value_free(challenge_json);
		goto finalize;
	}

	cached = getBool(challenge_json, "cached");
	info = getString(challenge_json, "info");

	if (!cached)
	{
		session_id = getString(challenge_json, "session_id");
		challenge = getString(challenge_json, "challenge");
	}
	json_value_free(challenge_json);

	/* Login was cached, continue successful */
	if (cached)
	{
		log_message(LOG_INFO, "%s", info);
		pam_result = PAM_SUCCESS;
		goto finalize;
	}

	/* Now we need to have a session_id and a challenge ! */
	if (!session_id || !challenge)
	{
		tty_output(pamh, SERVER_ERROR);
		goto finalize;
	}

	/* Show challenge URL... (user has to follow link !) */
	tty_output(pamh, challenge);

	bool timeout = false;

	/* Now User has to return to the prompted and anter the correct CODE !... */
	for (unsigned retry = 0; (retry < cfg->retries) &&
							 (pam_result != PAM_SUCCESS) &&
							 !timeout;
		 ++retry)
	{
		char *code = tty_input(pamh, PROMPT_CODE, PAM_PROMPT_ECHO_OFF);

		/* Prepare URL... */
		url = str_printf("%s/%s", cfg->url, API_CHECK_CODE_PATH);

		/* Prepare check code input data... */
		data = str_printf("{\"session_id\":\"%s\",\"pin\":\"%s\"}", session_id, code);
		free(code);

		/* Request check code result */
		json_char *verify_response = (json_char *) API(
			url,
			"POST",
			(char *[]){ "Content-Type: application/json", authorization, NULL},
			data
		);
		free(url);
		free(data);

		/* Something went wrong on the server */
		if (verify_response == NULL)
		{
			log_message(LOG_ERR, SERVER_ERROR);
			tty_output(pamh, SERVER_ERROR);
			pam_result = PAM_SYSTEM_ERR;
			break;
		}

		/* Parse auth result */
		json_value *verify_json = json_parse(verify_response, strnlen(verify_response, BUFSIZ));
		free(verify_response);

		/* See if the response contains an error */
		error = getBool(verify_json, "error");
		if (error)
		{
			/* See if the response contains a message */
			message = getString(verify_json, "message");
			if (message != NULL)
			{
				log_message(LOG_INFO, "%s", message);
				tty_output(pamh, message);
			} else
			{
				log_message(LOG_INFO, "%s", SERVER_NO_MESSAGE);
				tty_output(pamh, SERVER_NO_MESSAGE);
			}
			pam_result = PAM_SYSTEM_ERR;
			break;
		}

		char *result = getString(verify_json, "result");
		info = getString(verify_json, "info");
		json_value_free(verify_json);

		if (info)
		{
			tty_output(pamh, info);
			log_message(LOG_INFO, "info: %s\n", info);
		}

		if (result)
		{
			log_message(LOG_INFO, "result: %s\n", result);
			pam_result = !strcmp(result, "SUCCESS") ? PAM_SUCCESS : PAM_AUTH_ERR;
			timeout = !strcmp(result, "TIMEOUT");
			free(result);
		}

	}

finalize:
	if (info!=NULL)
		free(info);
	if (authorization!=NULL)
		free(authorization);
	if (challenge!=NULL)
		free(challenge);
	if (session_id!=NULL)
		free(session_id);
	if (message!=NULL)
		free(message);
	freeConfig(cfg);

	return pam_result;
}
