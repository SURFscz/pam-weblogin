#ifndef CONFIG_H
#define CONFIG_H
#include <stdbool.h>

#define DEFAULT_CACHE_DURATION 60
#define DEFAULT_RETRIES 1

typedef struct
{
	char *url;
	char *token;
	char *attribute;
	bool pam_user;
	unsigned int cache_duration;
	bool cache_per_rhost;
	unsigned int retries;
} Config;

void freeConfig(Config *cfg);
Config *getConfig(const char *filename);

#endif /* CONFIG_H */
