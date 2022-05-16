#include "defs.h"
#include "utils.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXLINE 1024

static char *trim(char *s)
{
	while (isspace(*(s + strlen(s) - 1)))
		*(s + strlen(s) - 1) = '\0';

	while (isspace(*s))
		s++;

	return s;
}

/*
 * freeConfig frees dynamic allocated memory
 */

void freeConfig(Config *cfg)
{
	if (cfg)
	{
		if (cfg->url)
			free(cfg->url);
		if (cfg->token)
			free(cfg->token);
		if (cfg->attribute)
			free(cfg->attribute);

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

Config *getConfig(const char *filename)
{
	int lineno = 0;
	FILE *fp = NULL;

	Config *cfg = malloc(sizeof(Config));
	if (cfg == NULL)
	{
		log_message(LOG_ERR, "Can't allocate memory");
		return NULL;
	}

	cfg->url = NULL;
	cfg->token = NULL;
	cfg->attribute = NULL;
	cfg->cache_duration = 60;
	cfg->retries = 1;

	if ((fp = fopen(filename, "r")) != NULL)
	{
		char buffer[MAXLINE];

		while (!feof(fp))
		{
			memset(buffer, 0, MAXLINE);

			fgets(buffer, MAXLINE, fp);

			lineno++;

			char *key = trim(buffer);

			/* Line starts with a # comment */
			if (key[0] == '#')
			{
				continue;
			}

			/* Line contains = token */
			char *val = strchr(key, '=');
			if (val == NULL)
			{
				//log_message(LOG_INFO, "Configuration line: %d: missing '=' symbol, skipping line", lineno);
				continue;
			}

			*val++ = '\0';

			val = trim(val);
			key = trim(key);

			/* Check for url config */
			if (!strcmp(key, "url"))
			{
				cfg->url = strdup(val);
				log_message(LOG_DEBUG, "url: %s", cfg->url);
			}

			/* Check for token config */
			if (!strcmp(key, "token"))
			{
				cfg->token = strdup(val);
				log_message(LOG_DEBUG, "token: %s", cfg->token);
			}

			/* Check for token config */
			if (!strcmp(key, "attribute"))
			{
				cfg->attribute = strdup(val);
				log_message(LOG_DEBUG, "attribute: %s", cfg->attribute);
			}

			/* Check for cache_duration config */
			if (!strcmp(key, "cache_duration"))
			{
				cfg->cache_duration = abs((int)strtol(val, NULL, 10));
				log_message(LOG_DEBUG, "cache_duration: %d", cfg->cache_duration);
			}

			/* Check for retries config */
			if (!strcmp(key, "retries"))
			{
				cfg->retries = abs((int)strtol(val, NULL, 10));
				log_message(LOG_DEBUG, "retries: %d", cfg->retries);
			}
		}
		fclose(fp);
	}

	/* Success if url, token and attribute are set */
	if (cfg->url && cfg->token && cfg->attribute)
	{
		return cfg;
	}

	// Fail if either url or token is unset
	if (!cfg->url)
		log_message(LOG_ERR, "Missing 'url' in configuration!");

	if (!cfg->token)
		log_message(LOG_ERR, "Missing 'token' in configuration!");

	if (!cfg->attribute)
		log_message(LOG_ERR, "Missing 'attribute' in configuration!");

	freeConfig(cfg);
	return NULL;
}
