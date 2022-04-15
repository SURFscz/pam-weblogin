#include "json.h"
#include <security/pam_appl.h>
#include <security/pam_modules.h>

#define PAM_CONST const

void log_message(int priority, pam_handle_t *pamh,
                        const char *format, ...);

// JSON helpers
json_value *findKey(pam_handle_t *pamh, json_value* value, const char* name);
char *getString(pam_handle_t *pamh, json_value*, const char* name);
bool getBool(pam_handle_t *pamh, json_value* value, const char* name);

static int converse(pam_handle_t *pamh, int nargs,
                    PAM_CONST struct pam_message **message,
                    struct pam_response **response);
char *conv_read(pam_handle_t *pamh,const char *text,int echocode);
void conv_info(pam_handle_t *pamh, const char* text);
