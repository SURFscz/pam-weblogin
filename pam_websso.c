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
    int retval;
    char* result;
    char url[URL_LEN];
    char data[DATA_LEN];

    json_char* json;
    json_value* value;

    const char* pUsername;
    retval = pam_get_user(pamh, &pUsername, "Username: ");

    // Prepare full url...
    strncpy(url, "http://localhost:5001/req", URL_LEN);
    printf("\nURL: %s\n", url);

    // Prepare input data...
    snprintf(data, DATA_LEN,
        "{\"user\":\"%s\"}",
        "martin"
    );

    result = fetchURL(url, data);

    json = (json_char*) result;
    value = json_parse(json, strlen(json));

    char* nonce;
    char* pin;
    char* challenge;
    bool hot;

    nonce = value->u.object.values[0].value->u.string.ptr;
    pin = value->u.object.values[1].value->u.string.ptr;
    challenge = value->u.object.values[2].value->u.string.ptr;
    hot = value->u.object.values[3].value->u.boolean;

    printf("nonce: %s\n", nonce);
    printf("pin: %s\n", pin);
    printf("challenge %s\n", challenge);
    printf("hot: %s\n", hot ? "true" : "false");

    // Prepare full url...
    strncpy(url, "http://localhost:5001/auth", URL_LEN);
    printf("\nURL: %s\n", url);

    // Prepare input data...
    snprintf(data, DATA_LEN,
        "{\"nonce\":\"%s\"}",
        "1234"
    );

    result = fetchURL(url, data);

    char* user;
    char* auth_result;
    json = (json_char*) result;
    value = json_parse(json, strlen(json));

    user = value->u.object.values[0].value->u.string.ptr;
    auth_result = value->u.object.values[1].value->u.string.ptr;

    printf("user: %s\n", user);
    printf("auth_result: %s\n", auth_result);

    return PAM_SUCCESS;
}
