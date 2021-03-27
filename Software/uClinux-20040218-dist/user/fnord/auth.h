#ifndef AUTH_H
#define AUTH_H

void auth_add(char *directory,char *file);
void auth_check(void);
int auth_authorize(const char *host, const char *url, const char *remote_ip_addr, const char *authorization, char username[15]);
void dump_auth(void);

#endif
