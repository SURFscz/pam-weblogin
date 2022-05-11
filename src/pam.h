#ifndef PAM_H
#define PAM_H

#include "defs.h"

#include <security/pam_appl.h>
#include <security/pam_modules.h>


#define PAM_CONST const

void log_message(int priority, pam_handle_t *pamh, const char *format, ...)
     __attribute__ ((format (printf, 3, 4)));

char *conv_read(pam_handle_t *pamh, const char *text, int echocode);
void conv_info(pam_handle_t *pamh, const char *text);

#endif
