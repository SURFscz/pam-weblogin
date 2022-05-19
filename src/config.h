#ifndef CONFIG_H
#define CONFIG_H
typedef struct
{
	char *url;
	char *token;
	char *attribute;
	unsigned int cache_duration;
	unsigned int retries;
} Config;

void freeConfig(Config *cfg);
Config *getConfig(const char *filename);

#endif // CONFIG_H
