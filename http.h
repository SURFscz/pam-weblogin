typedef enum {
  S_NOK,
  S_OK
} FETCHR;

FETCHR fetchURL(const char* url, const char* data, char** result);
