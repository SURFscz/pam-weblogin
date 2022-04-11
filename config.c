#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

#define MAXLINE 1024

char* trim(char s[]) {
  int i=0, j=0;

  char trim[MAXLINE]="\0";

  for (i=strlen(s)-1; s[i]==' ' || s[i]=='\n'; i--) {
      s[i]='\0';
  }

  for(i=0; s[i]==' '; i++);

  while (s[i]!='\0') {
      trim[j]=s[i];
      i++;
      j++;
  }

  strcpy(s,trim);

  return s;
}


/*
 * getConfig reads config file
 * key value pairs are separated by =, whitespace is removed
 * file may contain comments
 *
 * # The url to connect to
 * url = http://test.example.com/test
 */

int getConfig(const char* filename, Config** cfgp) {
    int rc = 0;
    FILE* fp ;
    char line[MAXLINE];
    char key[MAXLINE/2];
    char val[MAXLINE/2];

    (*cfgp)->url = NULL;
    (*cfgp)->token = NULL;

    printf("config: %s\n", filename);

    if ((fp = fopen(filename, "r")) != NULL) {
    while (! feof(fp)){
      memset(line, 0, MAXLINE);
      memset(key, 0, MAXLINE/2);
      memset(val, 0, MAXLINE/2);
      fgets(line, MAXLINE, fp);

      char* trimmed_line = trim(line);

      // Line starts with a # comment
      if (trimmed_line[0] == '#') {
        continue;
      }

      // Line contains = token
      char* pos = strchr(trimmed_line, '=');
      if (pos == NULL)
        continue;

      int len = strlen(trimmed_line);

      strncpy(key, trimmed_line, pos - trimmed_line);
      char* trimmed_key = trim(key);

      strncpy(val, pos+1, trimmed_line + len - pos);
      char* trimmed_val = trim(val);

      if (! strcmp(trimmed_key, "url")) {
        (*cfgp)->url = malloc(strlen(trimmed_val));
        strcpy((*cfgp)->url, trimmed_val);
        printf("config '%s' -> '%s'\n", trimmed_key, trimmed_val);
      }

      if (! strcmp(trimmed_key, "token")) {
        (*cfgp)->token = malloc(strlen(trimmed_val));
        strcpy((*cfgp)->token, trimmed_val);
        printf("config '%s' -> '%s'\n", trimmed_key, trimmed_val);
      }
    }

    if ((*cfgp)->url) {
      rc = 1;
    } else {
      printf("Error reading config\n");
    }

  } else {
      printf("Error reading config\n");
  }

  return rc;
}
