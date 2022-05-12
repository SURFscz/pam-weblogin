#include "defs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <security/pam_appl.h>
#include <security/pam_modules.h>

#include <json.h>

#include "utils.h"
#include "config.h"
#include "http.h"

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

	active_pamh = pamh;

	log_message(LOG_INFO, "Start of pam_websso");

	// Read username
	const char *username;
	if (pam_get_user(pamh, &username, "Username: ") != PAM_SUCCESS)
	{
		log_message(LOG_ERR, "Error getting user");
		return PAM_SYSTEM_ERR;
	}

	// Read configuration file
	Config *cfg = NULL;
	if (!(cfg = getConfig((argc > 0) ? argv[0] : "/etc/pam-websso.conf")))
	{
		conv_info(pamh, "Error reading conf");
		return PAM_SYSTEM_ERR;
	}
	/*
		log_message(LOG_INFO, "cfg->url: '%s'\n", cfg->url);
		log_message(LOG_INFO, "cfg->token: '%s'\n", cfg->token);
		log_message(LOG_INFO, "cfg->attribute: '%s'\n", cfg->attribute);
		log_message(LOG_INFO, "cfg->cache_duration: '%d'\n", cfg->cache_duration);
		log_message(LOG_INFO, "cfg->retries: '%d'\n", cfg->retries);
	*/

	asprintf(&authorization, "Authorization: %s", cfg->token);

	// Prepare full start url...
	char *url = NULL;
	asprintf(&url, "%s/start", cfg->url);

	// Prepare start input data...
	char *data = NULL;
	asprintf(&data, "{\"user_id\":\"%s\",\"attribute\":\"%s\",\"cache_duration\":\"%d\"}",
			 username, cfg->attribute, cfg->cache_duration);

	// Request auth session_id/challenge
	json_char *challenge_response = (json_char *) API(
		url,
		"POST",
		(char *[]){ "Content-Type: application/json", authorization, NULL},
		data
	);
	free(url);
	free(data);

	if (challenge_response == NULL) {
		log_message(LOG_ERR, "Error making request");
		conv_info(pamh, "Could not contact auth server");
		pam_result = PAM_SYSTEM_ERR;
		goto finalize;
	}

	// Parse response
	json_value *challenge_json = json_parse(challenge_response, strlen(challenge_response));
	free(challenge_response);

	session_id = getString(challenge_json, "session_id");
	challenge = getString(challenge_json, "challenge");
	cached = getBool(challenge_json, "cached");
	json_value_free(challenge_json);

	// /*
		log_message(LOG_INFO, "session_id: %s\n", session_id);
		log_message(LOG_INFO, "challenge: %s\n", challenge);
		log_message(LOG_INFO, "cached: %s\n", cached ? "true" : "false");
	// */

	// The answer didn't contain a session_id, no need to continue
	if (session_id == NULL)
	{
		conv_info(pamh, "Server error!");
		goto finalize;
	}

	// Login was cached, continue successful
	if (cached)
	{
		conv_info(pamh, "You were cached!");
		pam_result = PAM_SUCCESS;
		goto finalize;
	}

	/* Pin challenge Conversation */
	conv_info(pamh, challenge);
	
	bool timeout = false;

	for (unsigned retry = 0; (retry < cfg->retries) &&
							 (pam_result != PAM_SUCCESS) &&
							 !timeout;
		 ++retry)
	{
		char *pin = conv_read(pamh, "Pin: ", PAM_PROMPT_ECHO_OFF);

		/* Prepare URL... */
		asprintf(&url, "%s/check-pin", cfg->url);

		/* Prepare check pin input data... */
		asprintf(&data, "{\"session_id\":\"%s\",\"rpin\":\"%s\"}", session_id, pin);
		free(pin);

		/* Request check pin result */
		json_char *verify_response = (json_char *) API(
			url,
			"POST",
			(char *[]){ "Content-Type: application/json", authorization, NULL},
			data
		);
		free(url);
		free(data);

		if (verify_response == NULL) {
			log_message(LOG_ERR, "Error making request");
			conv_info(pamh, "Could not contact auth server");
			pam_result = PAM_SYSTEM_ERR;
			break;
		}
		/* Parse auth result */
		json_value *verify_json = json_parse(verify_response, strlen(verify_response));
		free(verify_response);

		char *result = getString(verify_json, "result");
		char *debug_msg = getString(verify_json, "debug_msg");
		json_value_free(verify_json);

		if (debug_msg)
		{
			conv_info(pamh, debug_msg);
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
	free(authorization);
	free(challenge);
	free(session_id);
	freeConfig(cfg);

	return pam_result;
}
