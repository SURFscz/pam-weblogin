#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

#define MAXLINE 1024

/*
 * getConfig reads config file
 * key value pairs are separated by =, whitespace is removed
 * file may contain comments
 *
 * # The url to connect to
 * url = http://test.example.com/test
 */

RCONFIG getConfig(const char* filename, Config** cfgp) {
  RCONFIG rc= C_NOK;
  FILE* fp ;
  char line[MAXLINE];
  char key[MAXLINE/2];
  char val[MAXLINE/2];

//   printf("Opening %s\n", filename);
  if ((fp = fopen(filename, "r")) != NULL) {
    while (! feof(fp)){
      memset(line, 0, MAXLINE);
      fgets(line, MAXLINE, fp);

      // Find first character of the line
      char* trimmed_line = line;
      while (trimmed_line[0] == ' ') {
          trimmed_line++;
      }

      // Line starts with a # comment
      if (trimmed_line[0] == '#') {
        continue;
      }

      char* pos = strchr(line, '=');
      if (pos == NULL)
        continue;

      int len = strlen(line);
      int offset = 1;
      if (line[len - 1] == '\n')
        offset = 2;

      memset(key, 0, MAXLINE/2);
      memset(val, 0, MAXLINE/2);

      strncpy(key, line, pos - line);
      char* trimmed_key = strtok(key, "\r\n\t ");

      if (! strcmp(trimmed_key, "url")) {
        strncpy(val, pos+1, line + len - offset - pos);
        char* trimmed_val = strtok(val, "\r\n\t ");
        (*cfgp)->url = malloc(strlen(trimmed_val));
        strcpy((*cfgp)->url, trimmed_val);
        printf("'%s' -> '%s'\n", trimmed_key, trimmed_val);
      } else {
        (*cfgp)->url = NULL;
      }

      if ((*cfgp)->url) {
        rc = C_OK;
      } else {
        printf("Error reading config\n");
      }
    }

  } else {
      printf("Error reading config\n");
  }

  return rc;
}
