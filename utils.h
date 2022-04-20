#ifndef _UTILS_H
#define _UTILS_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdbool.h>
#include <syslog.h>

#include <security/pam_appl.h>
#include <security/pam_modules.h>

#include "json.h"

#define PAM_CONST const

#ifndef asprintf
extern int asprintf(char **restrict strp, const char *restrict fmt, ...);
#endif

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

#endif