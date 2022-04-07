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
#define RESULT_LEN 1024

/* expected hook */
PAM_EXTERN int pam_sm_setcred( pam_handle_t *pamh, int flags, int argc, const char **argv ) {
    return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    printf("Acct mgmt\n");
    return PAM_SUCCESS;
}

/* expected hook, this is where custom stuff happens */
PAM_EXTERN int pam_sm_authenticate( pam_handle_t *pamh, int flags,int argc, const char **argv ) {
    RCONFIG cresult;
//     const char filename[] = "pam-websso.conf";
    const char* filename = "pam-websso.conf";
    if (argc > 0) {
      filename = argv[0];
    }
    printf("filename: %s\n", filename);

    // Read configuration file
    Config* cfg = malloc(sizeof(*cfg));
    cresult = getConfig(filename, &cfg);
    if (cresult == C_NOK) {
      printf("Error reading conf\n");
      return PAM_AUTH_ERR;
    }
    printf("config url: %s\n", cfg->url);

    int uresult;
    const char* username;

    char url[URL_LEN];
    char data[DATA_LEN];

    RPOST presult;
    char* req;
    char* auth;

    json_char* json;
    json_value* value;


    char* user;
    char* auth_result;

    uresult = pam_get_user(pamh, &username, "Username: ");

    // Prepare full url...
    snprintf(url, URL_LEN,
             "%s/req",
             cfg->url
    );

    printf("URL: %s\n", url);

    // Prepare input data...
    snprintf(data, DATA_LEN,
        "{\"user\":\"%s\"}",
        username
    );

    presult = postURL(url, data, &req);
    if (presult == P_NOK) {
      printf("Error making request\n");
      return PAM_AUTH_ERR;
    }
    printf("req: %s\n", req);

    json = (json_char*) req;
    value = json_parse(json, strlen(json));
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
    struct pam_response *resp;
    char *rpin;
    int pam_err, retry;

    pam_err = pam_get_item(pamh, PAM_CONV, &ptr);
    if (pam_err != PAM_SUCCESS)
        return PAM_SYSTEM_ERR;

    conv = ptr;
    msg.msg_style = PAM_PROMPT_ECHO_OFF;
    msg.msg = challenge;
    msgp = &msg;
    rpin = NULL;
    resp = NULL;

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
    }

    // Prepare full url...
    snprintf(url, URL_LEN,
             "%s/auth",
             cfg->url
    );
    printf("URL: %s\n", url);

    // Prepare input data...
    snprintf(data, DATA_LEN,
        "{\"nonce\":\"%s\"}",
        nonce
    );

    presult = postURL(url, data, &auth);
    if (presult == P_NOK) {
      printf("Error making request\n");
      return PAM_AUTH_ERR;
    }
    printf("auth: %s\n", auth);

    json = (json_char*) auth;
    value = json_parse(json, strlen(json));
    free(auth);

    user = value->u.object.values[0].value->u.string.ptr;
    auth_result = value->u.object.values[1].value->u.string.ptr;

    printf("user: %s\n", user);
    printf("auth_result: %s\n", auth_result);

    return PAM_SUCCESS;
}
