#ifndef _HTTP_H
#define _HTTP_H

#include "defs.h"

int postURL(const char* url, const char* token, const char* data, char** result);

#endif