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
//     printf("Authenticate\n");
    // Prepare Conversation(s)
    const void *ptr;
    const struct pam_conv *conv;
    int pam_err;
    struct pam_response* resp = NULL;

    struct pam_message msg;
    const struct pam_message *msgp;
    msgp = &msg;
    msg.msg_style = PAM_TEXT_INFO;

    pam_err = pam_get_item(pamh, PAM_CONV, &ptr);
    if (pam_err != PAM_SUCCESS)
        return PAM_SYSTEM_ERR;
    conv = ptr;

    const char *filename = "/etc/pam-websso.conf";
    if (argc > 0) {
        filename = argv[0];
    }

    // Read configuration file
    Config *cfg = malloc(sizeof(*cfg));
    if (! getConfig(filename, &cfg)) {
//         printf("Error reading conf\n");
        msg.msg = "Error reading conf";
        (*conv->conv)(1, &msgp, &resp, conv->appdata_ptr);
        return PAM_SYSTEM_ERR;
    }
/*
    printf("cfg->url: '%s'\n", cfg->url);
    printf("cfg->token: '%s'\n", cfg->token);
    printf("cfg->retries: '%d'\n", cfg->retries);
*/
    // Read username
    const char *username;
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
    char *req;
    if (! postURL(url, cfg->token, data, &req)) {
//         printf("Error making request\n");
//         msg.msg_style = PAM_TEXT_INFO;
        msg.msg = "Error making request";
        (*conv->conv)(1, &msgp, &resp, conv->appdata_ptr);
        return PAM_SYSTEM_ERR;
    }

//     printf("req: %s\n", req);

    // Parse response
    json_char *json = (json_char*) req;
    json_value *value = json_parse(json, strlen(json));
    free(req);

    char *nonce = getString(value, "nonce");
//     char *pin = getString(value, "pin");
    char *challenge = getString(value, "challenge");
    bool hot = getBool(value, "hot");
/*
    printf("nonce: %s\n", nonce);
    printf("pin: %s\n", pin);
    printf("challenge: %s\n", challenge);
    printf("hot: %s\n", hot ? "true" : "false");
*/
    if (hot) {
//         msg.msg_style = PAM_TEXT_INFO;
        msg.msg = "You are hot!";
        (*conv->conv)(1, &msgp, &resp, conv->appdata_ptr);
        return PAM_SUCCESS;
    }

    // Pin challenge Conversation
    msg.msg = challenge;
    (*conv->conv)(1, &msgp, &resp, conv->appdata_ptr);

    for (int retry = 0; retry < cfg->retries; ++retry) {
        msg.msg_style = PAM_PROMPT_ECHO_OFF;
        msg.msg = "pin: ";
        pam_err = (*conv->conv)(1, &msgp, &resp, conv->appdata_ptr);
        if (pam_err != PAM_SUCCESS) {
            return PAM_AUTH_ERR;
        }

        // From here we will only inform
        msg.msg_style = PAM_TEXT_INFO;

        char* rpin = NULL;
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

    /*
        printf("Pin: %s\n", rpin);
        if (! strcmp(pin, rpin)) {
            printf("Pin matched!\n");
        } else {
    //         printf("Pin didn't match\n");
            msg.msg = "Pin didn't match";
            (*conv->conv)(1, &msgp, &resp, conv->appdata_ptr);
            return PAM_AUTH_ERR;
        }
    */
        // Prepare full auth url...
        snprintf(url, URL_LEN,
            "%s/auth",
            cfg->url
        );

        // Prepare auth input data...
        snprintf(data, DATA_LEN,
            "{\"nonce\":\"%s\",\"rpin\":\"%s\"}", nonce, rpin
        );

        // Request auth result
        char* auth;
        if (! postURL(url, cfg->token, data, &auth)) {
    //         printf("Error making request\n");
            msg.msg = "Error making request";
            (*conv->conv)(1, &msgp, &resp, conv->appdata_ptr);
            return PAM_SYSTEM_ERR;
        }
    //     printf("auth: %s\n", auth);

        // Parse auth result
        json = (json_char*) auth;
        value = json_parse(json, strlen(json));
        free(auth);

        char *user = getString(value, "uid");
        char *auth_result = getString(value, "result");
        char *auth_msg = getString(value, "msg");
    /*
        printf("user: %s\n", user);
        printf("auth_result: %s\n", auth_result);
        printf("auth_msg: %s\n", auth_msg);
    */
        // Check auth conditions
        if (!user || strcmp(username, user)) {
    //         printf("User didn't match\n");
    //         msg.msg = "User didn't match";
            (*conv->conv)(1, &msgp, &resp, conv->appdata_ptr);
            return PAM_AUTH_ERR;
        }

        // Inform the user about the auth result
        msg.msg = auth_msg;
        (*conv->conv)(1, &msgp, &resp, conv->appdata_ptr);

        if (!strcmp(auth_result, "SUCCESS")) {
            return PAM_SUCCESS;
        }
    }

    return PAM_AUTH_ERR;
}
