typedef struct {
  char* url;
} Config;

int getConfig(const char* filename, Config** cfgp);
