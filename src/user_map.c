/*
if  create /etc/security/user_map.conf with the desired mapping
=========================================================
#comments and empty lines are ignored
bastiaan: basvandervlies
dennis: joedoe
=========================================================

and usually end up in /var/log/secure file.
*/

#include "defs.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <syslog.h>


#include "config.h"
#include "tty.h"
#include "user_map.h"


int user_map(const char *name, Config *cfg)
{
  char buf[256];
  FILE *f;
  int line = 0;

  f= fopen(FILENAME, "r");
  if (f == NULL)
  {
    return 0;
  }

  while (fgets(buf, sizeof(buf), f) != NULL)
  {
    char *s= buf, *from, *to, *end_from, *end_to;
    int cmp_result;

    line++;
    log_message(LOG_DEBUG, "HvB '%s'n", buf);

    skip(isspace(*s));
    if (*s == '#' || *s == 0) continue;

    from= s;
    skip(isalnum(*s) || (*s == '_') || (*s == '.') || (*s == '-') ||
         (*s == '$') || (*s == '\\') || (*s == '/'));
    end_from= s;
    skip(isspace(*s));

    if (end_from == from || *s++ != ':')
    {
       log_message(LOG_ERR, "Syntax error at %s:%d", FILENAME, line);
       fclose(f);
       return 0;
    }

    skip(isspace(*s));
    log_message(LOG_DEBUG, "HvB char '%s'n", s);
    to= s;
    skip(isalnum(*s) || (*s == '_') || (*s == '.') || (*s == '-') ||
         (*s == '$'));
    end_to= s;

    *end_from= *end_to= 0;

    cmp_result= (strcmp(name, from) == 0);
    log_message(LOG_DEBUG, "Check if username '%s': %s\n",
                                    from, cmp_result ? "YES":"NO");

    if (cmp_result)
    {
      log_message(LOG_INFO, "HvB name: %s\n", name);
      log_message(LOG_INFO, "HvB from: %s\n", from);
      log_message(LOG_DEBUG,"User mapped as '%s'\n", to);
      cfg->username = strdup(to);
      fclose(f);
      return 1;
    }
    else
    {
      cfg->username = strdup(name);
    }
  }

  fclose(f);
  return 0;
}
