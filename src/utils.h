#ifndef UTILS_H
#define UTILS_H

#include "defs.h"

#include <stdio.h>
#include <stdbool.h>
#include <syslog.h>

#include <security/pam_appl.h>
#include <security/pam_modules.h>

#include <json.h>

#define PAM_CONST const

extern pam_handle_t *active_pamh;
void log_message(int priority, const char *fmt, ...);

// JSON helpers
json_value *findKey(json_value *value, const char *name);
char *getString(json_value *, const char *name);
bool getBool(json_value *value, const char *name);

char *conv_read(pam_handle_t *pamh, const char *text, int echocode);
void conv_info(pam_handle_t *pamh, const char *text);

#endif
