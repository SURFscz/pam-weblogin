#include "defs.h"

#include <errno.h>
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
	int pam_result = PAM_AUTH_ERR;
	char *pam_user = NULL;
	char *pam_group = NULL;

	log_message(LOG_INFO, "Start of pam_weblogin");

	/* Read username */
	const char *username;
	if (pam_get_user(pamh, &username, PROMPT_USERNAME) != PAM_SUCCESS)
	{
		log_message(LOG_ERR, "Error getting user");
		return PAM_SYSTEM_ERR;
	}

        /* Check if debug argument was given */
	if (argc == 2 && strcmp(argv[1], "debug") == 0)
	{
		setlogmask(LOG_UPTO(LOG_DEBUG));
	}
        else
	{
		setlogmask(LOG_UPTO(LOG_INFO));
	}

	log_message(LOG_DEBUG, "BUFSIZE: %d bytes", BUFSIZE);

	/* Read configuration file */
	Config *cfg = NULL;
	if (!(cfg = getConfig((argc > 0) ? argv[0] : DEFAULT_CONF_FILE)))
	{
		if (!(cfg = getConfig(ALTERNATE_CONF_FILE)))
		{
			tty_output(pamh, "Error reading conf");
			return PAM_SYSTEM_ERR;
		}
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
	if (!cfg->pam_user) // We check local user against remote user based on attribute
	{
		data = str_printf("{\"user_id\":\"%s\",\"attribute\":\"%s\",\"cache_duration\":\"%d\",\"GIT_COMMIT\":\"%s\",\"JSONPARSER_GIT_COMMIT\":\"%s\"}",
				username, cfg->attribute, cfg->cache_duration, TOSTR(GIT_COMMIT), TOSTR(JSONPARSER_GIT_COMMIT));
	} else // We get local user from remote user based on attribute
	{
		data = str_printf("{\"attribute\":\"%s\",\"cache_duration\":\"%d\",\"GIT_COMMIT\":\"%s\",\"JSONPARSER_GIT_COMMIT\":\"%s\"}",
				cfg->attribute, cfg->cache_duration, TOSTR(GIT_COMMIT), TOSTR(JSONPARSER_GIT_COMMIT));
	}

	/* Request auth session_id/challenge */
	json_char *challenge_response = (json_char *) API(
		url,
		"POST",
		(char *[]){ "Content-Type: application/json", authorization, NULL},
		data,
		pamh
	);
	free(url);
	free(data);

	/* Something went wrong on the server */
	if (challenge_response == NULL)
	{
		pam_result = PAM_SYSTEM_ERR;
		goto finalize;
	}

	/* Parse response */
	json_value *challenge_json = json_parse(challenge_response, strnlen(challenge_response, BUFSIZE));
	free(challenge_response);

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
			data,
			pamh
		);
		free(url);
		free(data);

		/* Something went wrong on the server */
		if (verify_response == NULL)
		{
			break;
		}

		/* Parse auth result */
		json_value *verify_json = json_parse(verify_response, strnlen(verify_response, BUFSIZE));
		free(verify_response);

		char *result = getString(verify_json, "result");
		info = getString(verify_json, "info");

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

		// In case of success, try to find username and group (CO)
		if (pam_result == PAM_SUCCESS) {
			pam_user = getString(verify_json, "username");
			if (pam_user)
			{
				log_message(LOG_INFO, "PAM User: %s", pam_user);
			} else {
				log_message(LOG_INFO, "No user received");
			}

			// We map API collaboration to posix group
			json_value *pam_groups = findKey(verify_json, "collaborations");
			if (pam_groups) // We received groups
			{
				unsigned int max_groups = pam_groups->u.object.length;
				if (max_groups > 1) // There's more than one co in the list
				{
					char *end;
					long group = 0;
					bool range_error;
					while (true) { // Keep asking until valid input received
						errno = 0;
						tty_output(pamh, MSG_GROUPS);
						for (unsigned int i=0; i < max_groups; i++) {
							char *name = getString(
											getIndex(pam_groups, i)
											, "name");
							tty_output(pamh, str_printf("  [%d] %s", i+1, name));
						}
						char *group_input = tty_input(pamh, PROMPT_GROUP, PAM_PROMPT_ECHO_ON);
						group = strtol(group_input, &end, 10);
						range_error = errno == ERANGE;
						if (group < 1 || group > max_groups || range_error)
						{
							tty_output(pamh, PROMPT_WRONG_NUMBER);
						} else
						{
							pam_group = getString(
											getIndex(pam_groups, (unsigned int)(group - 1))
											, "short_name");
							break;
						}
					}
				} else // max_groups <= 1;
				{
					pam_group = getString(
									getIndex(pam_groups, 0)
									, "short_name");
				}
				log_message(LOG_INFO, "PAM Group: %s", pam_group);
			} else // no pam_groups
			{
				log_message(LOG_INFO, "No groups received");
			}
		}

		json_value_free(verify_json);
	}

finalize:
	// Set env vars
	if (pam_result == PAM_SUCCESS)
	{
		char *env = NULL;
		if (pam_user != NULL)
		{
			env = str_printf("PAM_USER=%s", pam_user);
			if (pam_putenv(pamh, env) != PAM_SUCCESS)
			{
				log_message(LOG_ERR, "Failed to set %s\n", env);
			}
		}
		if (pam_group != NULL)
		{
			env = str_printf("PAM_GROUP=%s", pam_group);
			if (pam_putenv(pamh, env) != PAM_SUCCESS)
			{
				log_message(LOG_ERR, "Failed to set %s\n", env);
			}
		}
		if (env != NULL)
			free(env);
	}
	if (pam_user != NULL)
		free(pam_user);
	if (pam_group != NULL)
		free(pam_group);
	if (info!=NULL)
		free(info);
	if (authorization!=NULL)
		free(authorization);
	if (challenge!=NULL)
		free(challenge);
	if (session_id!=NULL)
		free(session_id);
	freeConfig(cfg);

	return pam_result;
}
