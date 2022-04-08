#include "json.h"

// Object key finder
json_value* findKey(json_value* value, const char* name);

// Object value finders
char* getString(json_value*, const char* name);
bool getBool(json_value* value, const char* name);
