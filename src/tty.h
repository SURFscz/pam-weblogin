#ifndef TTY_H
#define TTY_H

#include <syslog.h>

#include <security/pam_appl.h>
#include <security/pam_modules.h>

void log_message(int priority, const char *fmt, ...)
	__attribute__ ((format (printf, 2, 3)));

char *tty_input(pam_handle_t *pamh, const char *text, int echo_code);
void tty_output(pam_handle_t *pamh, const char *text);

#endif /* TTY_H */
