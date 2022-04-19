#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "utils.h"
#include "config.h"

#define MAXLINE 1024

char * trim(char *s) {

  while (isspace(*(s+strlen(s)-1)))
    *(s+strlen(s)-1) = '\0';

  while (isspace(*s))
    s++;

  return s;
}

/*
 * gefreeConfig frees dynamic allocatoed memory
 */

void freeConfig(Config *cfg) {
  if (cfg) {
    if (cfg->url)
      free(cfg->url);
    if (cfg->token)
      free(cfg->token);

    free(cfg);
  }
}

/*
 * getConfig reads config file
 * key value pairs are separated by =, whitespace is removed
 * file may contain comments
 *
 * # The url to connect to
 * url = http://test.example.com/test
 * token = Bearer client:verysecret
 */

Config * getConfig(pam_handle_t *pamh, const char* filename) {
  int rc = 0;
  int lineno = 0;
  FILE *fp = NULL;

  Config *cfg = malloc(sizeof(Config));

  cfg->url = NULL;
  cfg->token = NULL;

  if ((fp = fopen(filename, "r")) != NULL) {
    char buffer[MAXLINE];

    while (! feof(fp)) {
      memset(buffer, 0, MAXLINE);

      fgets(buffer, MAXLINE, fp);

      lineno++;

      char *key = trim(buffer);

      // Line starts with a # comment
      if (key[0] == '#') {
        continue;
      }

      // Line contains = token
      char* val = strchr(key, '=');
      if (val == NULL) {
        log_message(LOG_ERR, pamh, "Configuration line: %d: missing '=' symbol, skipping line", lineno);
        continue;
      }

      *val++ = '\0';

      val = trim(val);
      key = trim(key);

      // Check for url config
      if (! strcmp(key, "url")) {
        cfg->url = strdup(val);
      }

      // Check for token config
      if (! strcmp(key, "token")) {
        cfg->token = strdup(val);
      }

      cfg->retries = 1;
      // Check for retries config
      if (! strcmp(key, "retries")) {
        cfg->retries = atoi(val);
      }
    }
  }

  // Success if both url and token are set !
  if (cfg->url && cfg->token) {
    return cfg;
  }

  // Fail if either url or token is unset
  if (! cfg->url)
    log_message(LOG_ERR, pamh, "Missing 'url' in configuration !");

  if (! cfg->token)
    log_message(LOG_ERR, pamh, "Missing 'token' in configuration !");

  freeConfig(cfg);
  return NULL;
}
