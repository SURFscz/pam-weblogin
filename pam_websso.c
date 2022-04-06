#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>

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
    int uresult;
    const char* username;

    char url[URL_LEN];
    char data[DATA_LEN];

    FETCHR fresult;
    char* req;
    char* auth;

    json_char* json;
    json_value* value;

    char* nonce;
    char* pin;
    char* challenge;
    bool hot;

    char* user;
    char* auth_result;

    uresult = pam_get_user(pamh, &username, "Username: ");

    // Prepare full url...
    strncpy(url, "http://localhost:5001/req", URL_LEN);
    printf("\nURL: %s\n", url);

    // Prepare input data...
    snprintf(data, DATA_LEN,
        "{\"user\":\"%s\"}",
        username
    );

    fresult = fetchURL(url, data, &req);
    printf("req: %s\n", req);
    // TODO inspect fresult

    json = (json_char*) req;
    value = json_parse(json, strlen(json));
    free(req);

    nonce = value->u.object.values[0].value->u.string.ptr;
    pin = value->u.object.values[1].value->u.string.ptr;
    challenge = value->u.object.values[2].value->u.string.ptr;
    hot = value->u.object.values[3].value->u.boolean;
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
    if (!strcmp(pin, rpin)) {
        printf("Pin matched!");
    } else {
        printf("Pin didn't match");
    }
    // Prepare full url...
    strncpy(url, "http://localhost:5001/auth", URL_LEN);
    printf("\nURL: %s\n", url);

    // Prepare input data...
    snprintf(data, DATA_LEN,
        "{\"nonce\":\"%s\"}",
        "1234"
    );

    fresult = fetchURL(url, data, &auth);
    printf("auth: %s\n", auth);
    // TODO inspect fresult

    json = (json_char*) auth;
    value = json_parse(json, strlen(json));
    free(auth);

    user = value->u.object.values[0].value->u.string.ptr;
    auth_result = value->u.object.values[1].value->u.string.ptr;

    printf("user: %s\n", user);
    printf("auth_result: %s\n", auth_result);

    return PAM_SUCCESS;
}
