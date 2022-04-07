#include <stdbool.h>
#include <string.h>
#include "json.h"

json_value* findKey(json_value* value, const char* name) {
  if (value == NULL) {
    return NULL;
  }
  char* object_name;
  for (int x = 0; x < value->u.object.length; x++) {
//     printf("object[%d].name = %s\n", x, value->u.object.values[x].name);
    if (!strcmp(value->u.object.values[x].name, name)) {
      return value->u.object.values[x].value;
    }
  }
  return NULL;
}

char* getString(json_value* value, const char* name) {
  json_value* key = findKey(value, name);
  if (key == NULL) {
    return "";
  }
  return key->u.string.ptr;
}

bool getBool(json_value* value, const char* name) {
  json_value* key = findKey(value, name);
  if (key == NULL) {
    return false;
  }
  return key->u.boolean;
}
