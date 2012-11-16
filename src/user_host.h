#ifndef _USER_HOST_H_
#define _USER_HOST_H_

#include "common.h"

typedef struct user {
    char name[NAMELEN];
    char host[HOSTLEN];
    char nickname[NAMELEN];
    struct user *next;
} user;

extern int list_users();

#endif /* _USER_HOST_H_ */
