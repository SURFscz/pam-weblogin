#ifndef HTTP_H
#define HTTP_H

#define USER_AGENT "pam-weblogin version v1"

char *API(const char* url, const char *method, char *headers[], const char* data);

#endif /* HTTP_H */
