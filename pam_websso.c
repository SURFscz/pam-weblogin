#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>

#include "config.h"
#include "utils.h"
#include "http.h"
#include "json.h"

#define URL_LEN 1024
#define DATA_LEN 1024

/* expected hook */
PAM_EXTERN int pam_sm_setcred( pam_handle_t *pamh, int flags, int argc, const char **argv ) {
    printf("Setcred\n");
    return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    printf("Acct mgmt\n");
    return PAM_SUCCESS;
}

/* expected hook, this is where custom stuff happens */
PAM_EXTERN int pam_sm_authenticate( pam_handle_t *pamh, int flags,int argc, const char **argv ) {
    printf("Authenticate\n");

    const char* filename = "/etc/pam-websso.conf";
    if (argc > 0) {
        filename = argv[0];
    }

    // Read configuration file
    Config* cfg = malloc(sizeof(*cfg));
    if (! getConfig(filename, &cfg)) {
        printf("Error reading conf\n");
        return PAM_AUTH_ERR;
    }
    printf("cfg->url: '%s'\n", cfg->url);
    printf("cfg->token: '%s'\n", cfg->token);

    // Read username if necessary
    const char* username;
    int uresult = pam_get_user(pamh, &username, "Username: ");

    // Prepare full req url...
    char url[URL_LEN];
    snprintf(url, URL_LEN,
        "%s/req",
        cfg->url
    );

    // Prepare req input data...
    char data[DATA_LEN];
    snprintf(data, DATA_LEN,
        "{\"user\":\"%s\"}",
        username
    );

    // Request auth nonce/challenge
    char* req;
    if (! postURL(url, cfg->token, data, &req)) {
        printf("Error making request\n");
        return PAM_AUTH_ERR;
    }
    printf("req: %s\n", req);

    // Parse response
    json_char* json = (json_char*) req;
    json_value* value = json_parse(json, strlen(json));
    free(req);

    char* nonce = getString(value, "nonce");
    char* pin = getString(value, "pin");
    char* challenge = getString(value, "challenge");
    bool hot = getBool(value, "hot");
/*
    printf("nonce: %s\n", nonce);
    printf("pin: %s\n", pin);
    printf("challenge: %s\n", challenge);
    printf("hot: %s\n", hot ? "true" : "false");
*/
    if (hot)
        return PAM_SUCCESS;

    // Pin Conversation
    const void *ptr;
    const struct pam_conv *conv;
    struct pam_message msg;
    const struct pam_message *msgp;
    int pam_err;

    pam_err = pam_get_item(pamh, PAM_CONV, &ptr);
    if (pam_err != PAM_SUCCESS)
        return PAM_SYSTEM_ERR;

    conv = ptr;
    msg.msg_style = PAM_PROMPT_ECHO_OFF;
    msg.msg = challenge;
    msgp = &msg;
    char* rpin = NULL;
    struct pam_response* resp = NULL;

    pam_err = (*conv->conv)(1, &msgp, &resp, conv->appdata_ptr);

    if (resp != NULL) {
        if (pam_err == PAM_SUCCESS)
            rpin = resp->resp;
        else
            free(resp->resp);
        free(resp);
    }
    if (pam_err == PAM_CONV_ERR)
        return pam_err;
    if (pam_err != PAM_SUCCESS)
        return PAM_AUTH_ERR;

    printf("Pin: %s\n", rpin);
    if (! strcmp(pin, rpin)) {
        printf("Pin matched!\n");
    } else {
        printf("Pin didn't match\n");
        return PAM_AUTH_ERR;
    }


    // Prepare full auth url...
    snprintf(url, URL_LEN,
        "%s/auth",
        cfg->url
    );

    // Prepare auth input data...
    snprintf(data, DATA_LEN,
        "{\"nonce\":\"%s\"}",
        nonce
    );

    // Request auth result
    char* auth;
    if (! postURL(url, cfg->token, data, &auth)) {
        printf("Error making request\n");
        return PAM_AUTH_ERR;
    }
    printf("auth: %s\n", auth);

    // Parse auth result
    json = (json_char*) auth;
    value = json_parse(json, strlen(json));
    free(auth);

    char* user = value->u.object.values[0].value->u.string.ptr;
    char* auth_result = value->u.object.values[1].value->u.string.ptr;

    printf("user: %s\n", user);
    printf("auth_result: %s\n", auth_result);

    // Check auth conditions
    if (strcmp(username, user)) {
        printf("User didn't match\n");
        return PAM_AUTH_ERR;
    }
    if (strcmp(auth_result, "SUCCESS")) {
        printf("Auth didn't succeed\n");
        return PAM_AUTH_ERR;
    }

    return PAM_SUCCESS;
}
