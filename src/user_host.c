#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "user_host.h"

user_list ulist;

int list_users()
{
    user *cur_user = ulist.user_head;
    char ipaddr[INET_ADDRSTRLEN];
    int num = 1;
    printf("******************************************************************\n");
    printf(" %-3s  %-10s%-30s%-20s\n", "Num", "Name", "Host", "IP");
    while (cur_user != NULL) {
        if (inet_ntop(AF_INET, &(cur_user->useraddr.sin_addr), ipaddr,  \
                      INET_ADDRSTRLEN) == NULL) {
            continue;
        }

        printf("%3d   %-8s%-30s%-20s\n", num, cur_user->name, \
               cur_user->host, ipaddr);
        cur_user = cur_user->next;
        num++;
    }
    printf("******************************************************************\n");

    return num-1;
}

int add_user(command *com)
{
    user *new_user, *tmp;
    tmp = ulist.user_head;
    while (tmp != NULL) {
        if (!strcmp(tmp->host, com->sender_host)) {
            return 0;
        }
        tmp = tmp->next;
    }
    if (com->addr.sin_addr.s_addr == htonl(INADDR_ANY)) {
        return -1;
    }

    new_user = (user *)malloc(sizeof(struct user));
    memcpy(new_user->name, com->sender_name, sizeof(new_user->name));
    memcpy(new_user->host, com->sender_host, sizeof(new_user->host));
    memcpy(&new_user->useraddr, &com->addr, sizeof(new_user->useraddr));
    new_user->next = NULL;

    if (ulist.user_head == NULL) {
        ulist.user_head =  ulist.user_tail = new_user;
    }else {
        ulist.user_tail->next = new_user;
        ulist.user_tail = new_user;
    }
    
    return 0;
}

int del_user(command *com)
{
    user *pre_user, *cur_user;

    pre_user = cur_user = ulist.user_head;
    while (cur_user != NULL) {
        if (!strcmp(com->sender_host, cur_user->host)) {
            if (cur_user == ulist.user_head) {
                ulist.user_head = cur_user->next;
                cur_user->next = NULL;
                free(cur_user);
                break;
            }
            pre_user->next = cur_user->next;
            cur_user->next = NULL;
            free(cur_user);
            break;
        }
        pre_user = cur_user;
        cur_user = cur_user->next;
    }
    
    return 0;
}

