typedef enum {
  C_NOK,
  C_OK
} RCONFIG;

typedef struct {
  char* url;
} Config;

RCONFIG getConfig(const char* filename, Config** cfgp);
