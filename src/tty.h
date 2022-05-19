#ifndef TTY_H
#define TTY_H

#include <syslog.h>

#include <security/pam_appl.h>
#include <security/pam_modules.h>

extern void log_message(int priority, const char *fmt, ...);

extern char *tty_input(pam_handle_t *pamh, const char *text, int echocode);
extern void tty_output(pam_handle_t *pamh, const char *text);

#endif // TTY_H
