typedef struct {
  char* url;
  char* token;
  unsigned int retries;
} Config;

int getConfig(const char* filename, Config** cfgp);
