#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <security/pam_appl.h>
#include <security/pam_modules.h>

#include "utils.h"
#include "config.h"
#include "http.h"
#include "json.h"

#define URL_LEN 1024
#define DATA_LEN 1024

/* expected hook */
PAM_EXTERN int pam_sm_setcred( pam_handle_t *pamh, int flags, int argc, const char *argv[] ) {
    //printf("Setcred\n");
    return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char *argv[]) {
    //printf("Acct mgmt\n");
    return PAM_SUCCESS;
}

/* expected hook, this is where custom stuff happens */
PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags,int argc, const char *argv[] ) {
    log_message(LOG_INFO, pamh, "Start of pam_websso");

    // Read username
    const char *username;
    if (pam_get_user(pamh, &username, "Username: ") != PAM_SUCCESS) {
        log_message(LOG_ERR, pamh, "Error getting user");
        return PAM_SYSTEM_ERR;
    }

    // Read configuration file
    Config *cfg = NULL;
    if (! (cfg = getConfig(pamh, (argc > 0) ? argv[0] : "/etc/pam-websso.conf"))) {
        conv_info(pamh, "Error reading conf");
        return PAM_SYSTEM_ERR;
    }

    /*
    log_message(LOG_INFO, pamh, "cfg->url: '%s'\n", cfg->url);
    log_message(LOG_INFO, pamh, "cfg->token: '%s'\n", cfg->token);
    log_message(LOG_INFO, pamh, "cfg->retries: '%d'\n", cfg->retries);
    */

    // Prepare full req url...
    char url[URL_LEN];
    snprintf(url, URL_LEN,
        "%s/req", cfg->url
    );

    // Prepare req input data...
    char data[DATA_LEN];
    snprintf(data, DATA_LEN,
        "{\"user\":\"%s\"}", username
    );

    // Request auth nonce/challenge
    char *req;
    if (! postURL(url, cfg->token, data, &req)) {
        log_message(LOG_ERR, pamh, "Error making request");
        conv_info(pamh, "Could not contact auth server");
        freeConfig(cfg);
        return PAM_SYSTEM_ERR;
    }

    log_message(LOG_INFO, pamh, "req: %s", req);

    // Parse response
    json_char *json = (json_char*) req;
    json_value *value = json_parse(json, strlen(json));
    free(req);

    char *nonce = getString(pamh, value, "nonce");
    char *challenge = getString(pamh, value, "challenge");
    bool hot = getBool(pamh, value, "hot");
    free(value);
/*
    log_message(LOG_INFO, pamh, "nonce: %s\n", nonce);
    log_message(LOG_INFO, pamh, "challenge: %s\n", challenge);
    log_message(LOG_INFO, pamh, "hot: %s\n", hot ? "true" : "false");
*/
    if (hot) {
        conv_info(pamh, "You were hot!");
        freeConfig(cfg);
        return PAM_SUCCESS;
    }

    // Pin challenge Conversation
    conv_info(pamh, challenge);
    free(challenge);

    int retval = PAM_AUTH_ERR;

    for (int retry = 0; (retry < cfg->retries) && (retval != PAM_SUCCESS); ++retry) {
        char *rpin = conv_read(pamh, "Pin: ", PAM_PROMPT_ECHO_OFF);

        // Prepare full auth url...
        snprintf(url, URL_LEN,
            "%s/auth", cfg->url
        );

        // Prepare auth input data...
        snprintf(data, DATA_LEN,
            "{\"nonce\":\"%s\",\"rpin\":\"%s\"}", nonce, rpin
        );

        // Request auth result
        char* auth;
        if (! postURL(url, cfg->token, data, &auth)) {
            log_message(LOG_ERR, pamh, "Error making request");
            conv_info(pamh, "Could not contact auth server");
            freeConfig(cfg);
            return PAM_SYSTEM_ERR;
        }

        log_message(LOG_INFO, pamh, "auth: %s\n", auth);

        // Parse auth result
        json = (json_char*) auth;
        value = json_parse(json, strlen(json));
        free(auth);

        char *auth_result = getString(pamh, value, "result");
        char *auth_msg = getString(pamh, value, "msg");
        free(value);

        conv_info(pamh, auth_msg);

        log_message(LOG_INFO, pamh, "auth_result: %s\n", auth_result);
        log_message(LOG_INFO, pamh, "auth_msg: %s\n", auth_msg);

        retval = (auth_result && !strcmp(auth_result, "SUCCESS")) ? PAM_SUCCESS : PAM_AUTH_ERR;

        free(auth_result);
        free(auth_msg);
    }

    free(nonce);
    freeConfig(cfg);
    return retval;
}
