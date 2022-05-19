#ifndef HTTP_H
#define HTTP_H

extern char *API(const char* url, const char *method, char *headers[], const char* data, long expected_response_code);

#endif // HTTP_H
