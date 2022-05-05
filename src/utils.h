#ifndef _UTILS_H
#define _UTILS_H

#include "defs.h"

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
json_value *findKey(json_value* value, const char* name);
char *getString(json_value*, const char* name);
bool getBool(json_value* value, const char* name);

char *conv_read(pam_handle_t *pamh,const char *text,int echocode);
void conv_info(pam_handle_t *pamh, const char* text);


#endif
