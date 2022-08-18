#include "defs.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>

#include "tty.h"

#include "config.h"

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
	struct stat sb;
	long unsigned mode;

	Config *cfg = malloc(sizeof(Config));
	if (cfg == NULL)
	{
		log_message(LOG_ERR, "Can't allocate memory");
		return NULL;
	}

	cfg->url = NULL;
	cfg->token = NULL;
	cfg->attribute = NULL;
	cfg->cache_duration = DEFAULT_CACHE_DURATION;
	cfg->retries = DEFAULT_RETRIES;

	// Check filename access and permissions
	if (access(filename, F_OK) == -1)
	{
		log_message(LOG_INFO, "config not accessible");
		return NULL;
	}

	if (stat(filename, &sb) == -1)
	{
		log_message(LOG_INFO, "config stat error");
		return NULL;
	}

	mode = sb.st_mode & 0777;
	if (mode > 0600)
	{
		log_message(LOG_INFO, "config permissions too wide");
		return NULL;
	}

	if ((fp = fopen(filename, "r")) != NULL)
	{
		char buffer[MAXLINE];

		while (!feof(fp))
		{
			memset(buffer, 0, MAXLINE);

			if (fgets(buffer, MAXLINE, fp) == NULL) {
				log_message(LOG_ERR, "No more lines in: %s", filename);
				break;
			}

			lineno++;

			char *key = trim(buffer);

			/* Line starts with a # comment */
			if (key[0] == '#' || key[0] == '\0')
			{
				continue;
			}

			/* Line contains = token */
			char *val = strchr(key, '=');
			if (val == NULL)
			{
				log_message(LOG_INFO, "Configuration line: %d: missing '=' symbol, skipping line", lineno);
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
			else if (!strcmp(key, "token"))
			{
				cfg->token = strdup(val);
				log_message(LOG_DEBUG, "token: %s", cfg->token);
			}

			/* Check for token config */
			else if (!strcmp(key, "attribute"))
			{
				cfg->attribute = strdup(val);
				log_message(LOG_DEBUG, "attribute: %s", cfg->attribute);
			}

			/* Check for cache_duration config */
			else if (!strcmp(key, "cache_duration"))
			{
				long cache_duration = strtol(val, NULL, 10);
				if (   ( cache_duration == 0        && errno == EINVAL )
				    || ( cache_duration == LONG_MAX && errno == ERANGE )
					||   cache_duration < 0 )
				{
					log_message(LOG_INFO, "Configuration file line %d: parse error (%s)", lineno, strerror(errno));
					continue;
				}
				cfg->cache_duration = (unsigned int)labs(strtol(val, NULL, 10));
				log_message(LOG_DEBUG, "cache_duration: %d", cfg->cache_duration);
			}

			/* Check for retries config */
			else if (!strcmp(key, "retries"))
			{
				cfg->retries = (unsigned int)labs(strtol(val, NULL, 10));
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

	/* Fail if either url or token is unset */
	if (!cfg->url)
		log_message(LOG_ERR, "Missing 'url' in configuration!");

	if (!cfg->token)
		log_message(LOG_ERR, "Missing 'token' in configuration!");

	if (!cfg->attribute)
		log_message(LOG_ERR, "Missing 'attribute' in configuration!");

	freeConfig(cfg);
	return NULL;
}
