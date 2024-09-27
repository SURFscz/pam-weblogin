#include "defs.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <stdbool.h>

#include "tty.h"
#include "utils.h"

#include "config.h"

#define MAXLINE 1024

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

/* Check config file access and permissions */
static bool check_file(const char * const filename)
{
	struct stat sb;
	unsigned int mode;
	char buf[256];

	if (access(filename, R_OK) == -1)
	{
		log_message(LOG_ERR, "config file not accessible");
		return false;
	}

	if (stat(filename, &sb) == -1)
	{
		buf[0] = '\0';
		if (strerror_r(errno, buf, sizeof(buf)))
			log_message(LOG_ERR, "config file stat error %i: %s", errno, buf);
		else
			log_message(LOG_ERR, "unknown error while checking config file");
		return false;
	}

	if (!S_ISREG(sb.st_mode))
	{
		log_message(LOG_ERR, "config file is not a regular file");
		return false;
	}

	mode = sb.st_mode & 07777;
	/* this check is also safe for setfacl() &co, because the group bits
	   acts as masks for ACL operation */
	if (mode&07177)
	{
		log_message(LOG_ERR, "config file permissions too wide: %03o (expected 0600 or 0400)", mode);
		return false;
	}

	return true;
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
	cfg->cache_duration = DEFAULT_CACHE_DURATION;
	cfg->retries = DEFAULT_RETRIES;
	cfg->pam_user = false;

	if (!check_file(filename))
	{
		/* hah! Look mom, no hands! */
		goto fail;
	}

	if ((fp = fopen(filename, "r")) != NULL)
	{
		char buffer[MAXLINE];

		while (!feof(fp))
		{
			memset(buffer, 0, MAXLINE);

			/* after this, buffer is guaranteed to be \0-terminated */
			if (fgets(buffer, MAXLINE, fp) == NULL) {
				log_message(LOG_DEBUG, "No more lines in: %s", filename);
				break;
			}

			lineno++;

			char *key = trim(buffer, strlen(buffer)); /* strlen() is safe here */

			/* Line starts with a # comment */
			if (key[0] == '#' || key[0] == '\0')
			{
				continue;
			}

			/* Line contains = token */
			char *val = strchr(key, '=');
			if (val == NULL)
			{
				/* Check for bare pam_user config */
				if (!strcmp(key, "pam_user"))
				{
					cfg->pam_user = true;
					log_message(LOG_DEBUG, "pam_user");
				}
				else
				{
					log_message(LOG_INFO, "Configuration line: %d: missing '=' symbol, skipping line", lineno);
				}
				continue;
			}

			*val++ = '\0';

			key = trim(key, strlen(key)); /* strlen() is safe here */
			val = trim(val, strlen(val)); /* strlen() is safe here */

			/* Check for url config */
			if (!strcmp(key, "url"))
			{
				cfg->url = strdup(val);
				log_message(LOG_DEBUG, "url: %s", cfg->url);
			}

			/* Check for token config */
			else if (!strcmp(key, "token"))
			{
				char *multi = strchr(val, ' ');
				if (multi == NULL)
				/* token is single valued, add Bearer */
				{
					cfg->token = str_printf("Bearer %s", val);
				} else
				/* token is multi valued, use as-is */
				{
					cfg->token = strdup(val);
				}
				log_message(LOG_DEBUG, "token: %s", cfg->token);
			}

			/* Check for attribute config */
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

fail:
	freeConfig(cfg);
	return NULL;
}
