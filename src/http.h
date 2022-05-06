#ifndef HTTP_H
#define HTTP_H

#include "defs.h"

int postURL(const char* url, const char* token, const char* data, char** result);

#endif
