#ifndef _CONFIG_H
#define _CONFIG_H

#include "defs.h"

#include <security/pam_appl.h>

typedef struct {
  char *url;
  char *token;
  char *attribute;
  unsigned int cache_duration;
  unsigned int retries;
} Config;

void freeConfig(Config *cfg);
Config *getConfig(pam_handle_t *pamh, const char* filename);

#endif
