#ifndef USER_MAP_H
#define USER_MAP_H

#define FILENAME "/etc/security/user_map.conf"
#define skip(what) while (*s && (what)) s++

int  user_map(const char *name, Config *cfg);

#endif /* USER_MAP_H */
