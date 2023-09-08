#define _XOPEN_SOURCE 700
#define UNUSED __attribute__ ((unused))
#include <stdlib.h>
#include <string.h>
#include <security/pam_modules.h>

static int converse(pam_handle_t *pamh, int nargs,
                    const struct pam_message **message,
                    struct pam_response **response)
{
    struct pam_conv *conv;
    int retval = pam_get_item(pamh, PAM_CONV, (void *)&conv);
    if (retval != PAM_SUCCESS)
    {
        return retval;
    }
    return conv->conv(nargs, message, response, conv->appdata_ptr);
}

void tty_output(pam_handle_t *pamh, const char *text)
{
    /* note: on MacOS, pam_message.msg is a non-const char*, so we need to copy it */
    char * pam_msg = strdup(text);
    if (pam_msg==NULL)
    {
        return;
    }

    const struct pam_message msg = {
        .msg_style = PAM_TEXT_INFO,
        .msg = pam_msg
    };

    const struct pam_message *msgs = &msg;
    struct pam_response *resp = NULL;
    const int retval = converse(pamh, 1, &msgs, &resp);

    free(pam_msg);
    free(resp);
}

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
    char *info = "Info";
    tty_output(pamh, info);

    return PAM_SUCCESS;
}
