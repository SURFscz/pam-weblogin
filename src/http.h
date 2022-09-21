#ifndef HTTP_H
#define HTTP_H

#define USER_AGENT "pam-weblogin version v1"

#define SERVER_UNREACHABLE "Server unreachable!"
#define SERVER_ERROR "Server error!"
#define PERMISSION_DENIED "Permission denied!"

char *API(const char* url, const char *method, char *headers[], const char* data, pam_handle_t *pamh);

#endif /* HTTP_H */
