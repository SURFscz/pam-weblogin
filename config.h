typedef struct {
  char* url;
  char* token;
} Config;

int getConfig(const char* filename, Config** cfgp);
