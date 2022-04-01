#include <curl/curl.h>

#define URL_LEN 1024
#define DATA_LEN 1024

typedef enum {
  E_UNEXPECTED,
  S_OK
} CRESULT;

size_t curl_callback(void* contents, size_t size, size_t nmemb, void* userp);
CRESULT fetchURL();
