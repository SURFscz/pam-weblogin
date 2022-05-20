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

#include "pam_websso.h"

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
	char *authorization = NULL;
	bool cached = false;
	int pam_result = PAM_AUTH_ERR;

	log_message(LOG_INFO, "Start of pam_websso");

	// Read username
	const char *username;
	if (pam_get_user(pamh, &username, PROMPT_USERNAME) != PAM_SUCCESS)
	{
		log_message(LOG_ERR, "Error getting user");
		return PAM_SYSTEM_ERR;
	}

	// Read configuration file
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

	// Prepare full start url...
	char *url = NULL;
	url = str_printf("%s/%s", cfg->url, API_START_PATH);

	// Prepare start input data...
	char *data = NULL;
	data = str_printf("{\"user_id\":\"%s\",\"attribute\":\"%s\",\"cache_duration\":\"%d\"}",
			 username, cfg->attribute, cfg->cache_duration);

	// Request auth session_id/challenge
	json_char *challenge_response = (json_char *) API(
		url,
		"POST",
		(char *[]){ "Content-Type: application/json", authorization, NULL},
		data,
		API_START_RESPONSE_CODE
	);
	free(url);
	free(data);

	if (challenge_response == NULL) {
		log_message(LOG_ERR, "Error making request");
		tty_output(pamh, "Could not contact auth server");
		pam_result = PAM_SYSTEM_ERR;
		goto finalize;
	}

	// Parse response
	json_value *challenge_json = json_parse(challenge_response, strnlen(challenge_response, BUFSIZ));
	free(challenge_response);

	cached = getBool(challenge_json, "cached");

	if (!cached) {
		session_id = getString(challenge_json, "session_id");
		challenge = getString(challenge_json, "challenge");
	}
	json_value_free(challenge_json);

	// Login was cached, continue successful
	if (cached)
	{
		tty_output(pamh, "You were cached!");
		pam_result = PAM_SUCCESS;
		goto finalize;
	}

	// Now we need to have a session_id and a challenge !
	if (!session_id || !challenge)
	{
		tty_output(pamh, "Server error!");
		goto finalize;
	}

	/* Show challenge URL... (user has to follow link !) */
	tty_output(pamh, challenge);

	bool timeout = false;

	/* Now User has to return to the prompted and anter the correct PIN !... */
	for (unsigned retry = 0; (retry < cfg->retries) &&
							 (pam_result != PAM_SUCCESS) &&
							 !timeout;
		 ++retry)
	{
		char *pin = tty_input(pamh, PROMPT_PIN, PAM_PROMPT_ECHO_OFF);

		/* Prepare URL... */
		url = str_printf("%s/%s", cfg->url, API_CHECK_PIN_PATH);

		/* Prepare check pin input data... */
		data = str_printf("{\"session_id\":\"%s\",\"pin\":\"%s\"}", session_id, pin);
		free(pin);

		/* Request check pin result */
		json_char *verify_response = (json_char *) API(
			url,
			"POST",
			(char *[]){ "Content-Type: application/json", authorization, NULL},
			data,
			API_CHECK_PIN_RESPONSE_CODE
		);
		free(url);
		free(data);

		if (verify_response == NULL) {
			log_message(LOG_ERR, "Error making request");
			tty_output(pamh, "Could not contact auth server");
			pam_result = PAM_SYSTEM_ERR;
			break;
		}
		/* Parse auth result */
		json_value *verify_json = json_parse(verify_response, strnlen(verify_response, BUFSIZ));
		free(verify_response);

		char *result = getString(verify_json, "result");
		char *debug_msg = getString(verify_json, "debug_msg");
		json_value_free(verify_json);

		if (debug_msg)
		{
			tty_output(pamh, debug_msg);
			log_message(LOG_INFO, "debug_msg: %s\n", debug_msg);
			free(debug_msg);
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
	if (authorization!=NULL)
		free(authorization);
	if (challenge!=NULL)
		free(challenge);
	if (session_id!=NULL)
		free(session_id);
	freeConfig(cfg);

	return pam_result;
}
