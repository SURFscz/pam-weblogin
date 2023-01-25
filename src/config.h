#ifndef CONFIG_H
#define CONFIG_H

#define DEFAULT_CACHE_DURATION 60
#define DEFAULT_RETRIES 1

typedef struct
{
	char *url;
	char *token;
	char *attribute;
	char *username;
	unsigned int cache_duration;
	unsigned int retries;
} Config;

void freeConfig(Config *cfg);
Config *getConfig(const char *filename);

#endif /* CONFIG_H */
