typedef enum {
  P_NOK,
  P_OK
} RPOST;

RPOST postURL(const char* url, const char* data, char** result);
