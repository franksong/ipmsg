#ifndef _USER_HOST_H_
#define _USER_HOST_H_

#include "common.h"

typedef struct user {
    char name[NAMELEN];
    char host[HOSTLEN];
    struct sockaddr_in useraddr;
    struct user *next;
} user;

typedef struct user_list {
    user *user_head;
    user *user_tail;
} user_list;

extern user_list ulist;

extern int list_users();
extern int add_user(command *com);
extern int del_user(command *com);

#endif /* _USER_HOST_H_ */
