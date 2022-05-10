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

/* expected hook */
PAM_EXTERN int pam_sm_setcred(UNUSED pam_handle_t *pamh, UNUSED int flags, UNUSED int argc, UNUSED const char *argv[])
{
	// printf("Setcred\n");
	return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_acct_mgmt(UNUSED pam_handle_t *pamh, UNUSED int flags, UNUSED int argc, UNUSED const char *argv[])
{
	// printf("Acct mgmt\n");
	return PAM_SUCCESS;
}

/* expected hook, this is where custom stuff happens */
/* TODO: this function is too long; split it up */
PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, UNUSED int flags, int argc, const char *argv[])
{
	log_message(LOG_INFO, pamh, "Start of pam_websso");

	/* Read username */
	const char *username;
	if (pam_get_user(pamh, &username, "Username: ") != PAM_SUCCESS)
	{
		log_message(LOG_ERR, pamh, "Error getting user");
		return PAM_SYSTEM_ERR;
	}

	/* Read configuration file */
	/* TODO: shouldn't we do this first? */
	Config *cfg = NULL;
	/* TODO: let's actually check argv[0] to see if it makes sense as a filename (getConfig doesn't do any checking) */
	if (!(cfg = getConfig(pamh, (argc > 0) ? argv[0] : "/etc/pam-websso.conf")))
	{
		conv_info(pamh, "Error reading conf");
		return PAM_SYSTEM_ERR;
	}
	/*
		log_message(LOG_INFO, pamh, "cfg->url: '%s'\n", cfg->url);
		log_message(LOG_INFO, pamh, "cfg->token: '%s'\n", cfg->token);
		log_message(LOG_INFO, pamh, "cfg->attribute: '%s'\n", cfg->attribute);
		log_message(LOG_INFO, pamh, "cfg->cache_duration: '%s'\n", cfg->cache_duration);
		log_message(LOG_INFO, pamh, "cfg->retries: '%d'\n", cfg->retries);
	*/
	/* Prepare full req url... */
	/* TODO: check return value */
	char *url = NULL;
	asprintf(&url, "%s/start", cfg->url);

	/* Prepare req input data... */
	/* TODO: check return value */
	char *data = NULL;
	asprintf(&data, "{\"user_id\":\"%s\",\"attribute\":\"%s\",\"cache_duration\":\"%d\"}",
			 username, cfg->attribute, cfg->cache_duration);

	/* Request auth session_id/challenge */
	// Request auth session_id/challenge
	char *start = NULL;
	int rc = postURL(url, cfg->token, data, &start);

	if (!rc)
	{
		log_message(LOG_ERR, pamh, "Error making request");
		conv_info(pamh, "Could not contact auth server");
		freeConfig(cfg);
		return PAM_SYSTEM_ERR;
	}

	free(url);
	free(data);

	log_message(LOG_INFO, pamh, "start: %s", start);

	/* Parse response */
	/* TODO:ugly, let's just make a wrapper function for this evil casting*/
	json_char *json = (json_char *)start;
	json_value *value = json_parse(json, strlen(json));
	free(start);

	char *session_id = getString(value, "session_id");
	char *challenge = getString(value, "challenge");
	bool cached = getBool(value, "cached");
	free(value);
	/*
		log_message(LOG_INFO, pamh, "session_id: %s\n", session_id);
		log_message(LOG_INFO, pamh, "challenge: %s\n", challenge);
		log_message(LOG_INFO, pamh, "cached: %s\n", cached ? "true" : "false");
	*/

	// The answer didn't contain a session_id, no need to continue
	if (!session_id)
	{
		conv_info(pamh, "Server error!");
		freeConfig(cfg);
		return PAM_AUTH_ERR;
	}

	// Login was cached, continue successful
	if (cached)
	{
		/* TODO: this is user facing stuff; create a def or something for this */
		conv_info(pamh, "You were cached!");
		freeConfig(cfg);
		free(session_id);
		free(challenge);
		return PAM_SUCCESS;
	}

	/* Pin challenge Conversation */
	conv_info(pamh, challenge);
	free(challenge);

	int retval = PAM_AUTH_ERR;
	bool timeout = false;

	for (unsigned retry = 0; (retry < cfg->retries) &&
	                         (retval != PAM_SUCCESS) &&
	                         !timeout;
	                         ++retry)
	{
		/* TODO: better variable name */
		/* TODO: check return value here! */
		char *pin = conv_read(pamh, "Pin: ", PAM_PROMPT_ECHO_OFF);

		/* Prepare URL... */
		asprintf(&url, "%s/check-pin", cfg->url);

		/* Prepare auth input data... */
		asprintf(&data, "{\"session_id\":\"%s\",\"pin\":\"%s\"}", session_id, pin);
		free(pin);

		/* Request auth result */
		char *auth = NULL;
		rc = postURL(url, cfg->token, data, &auth);
		free(url);
		free(data);

		if (!rc)
		{
			log_message(LOG_ERR, pamh, "Error making request");
			conv_info(pamh, "Could not contact auth server");
			retval = PAM_SYSTEM_ERR;
			break;
		}

		log_message(LOG_INFO, pamh, "auth: %s\n", auth);

		/* Parse auth result */
		/* TODO: see above, create wrapper function */
		json = (json_char *)auth;
		value = json_parse(json, strlen(json));
		free(auth);

		char *result = getString(value, "result");
		char *debug_msg = getString(value, "debug_msg");
		free(value);

		if (debug_msg)
		{
			/* TODO: sanitize this message before showing it to pam */
			conv_info(pamh, debug_msg);
			log_message(LOG_INFO, pamh, "debug_msg: %s\n", debug_msg);
			free(debug_msg);
		}

		if (result)
		{
			log_message(LOG_INFO, pamh, "result: %s\n", result);
			retval = !strcmp(result, "SUCCESS") ? PAM_SUCCESS : PAM_AUTH_ERR;
			timeout = !strcmp(result, "TIMEOUT");
			free(result);
		}

	}

	free(session_id);
	freeConfig(cfg);
	return retval;
}
