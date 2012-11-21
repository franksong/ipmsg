#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ipmsg.h"
#include "user_host.h"
#include "common.h"
#include "init.h"
#include "send_receive.h"

int send_msg(command *option, struct sockaddr_in *addr, socklen_t len)
{
    char sendbuf[MAXLEN];
    command *com = option;

    create_sendbuf(sendbuf, com);
    if (sendto(udp_sock, sendbuf, strlen(sendbuf)+1, 0, (struct sockaddr *)addr, \
               len) < 0) {
        printf("send_msg: sendto error.\n");
        return -1;
    }
    printf("sendbuf:%s\n", sendbuf); //debug
    
    return 0;
}

static int input_num(int min, int max, int defualt)
{
    int num;

    printf("Input num of user who you want to talk(cancel: 0.): ");
    while (1) {
        scanf("%d", &num);
        if (num == 0) {
            getchar();
            return 0;
        }

        if (num < min || num > max) {
            getchar();
            printf("no this user, please input again: ");
            continue;
        }
        getchar();
        return num;
    }

}

int talkto_user()
{
    char input_msg[MAXLEN];
    int who, count;
    user talk_user;
    user *ptruser;
    command sendcom;
    
    pthread_mutex_lock(&user_lock);
    count = list_users();
    pthread_mutex_unlock(&user_lock);

    who = input_num(1, count, 1);
    
    if (who == 0) {
        return 0;
    }
    pthread_mutex_lock(&user_lock);
    ptruser = ulist.user_head;
    who--;
    while (who) {
        ptruser = ptruser->next;
        who--;
    }
    memcpy(&talk_user, ptruser, sizeof(struct user));
    pthread_mutex_unlock(&user_lock);
    printf("\n");
    while (1) {
        printf("Talk to %s@%s(Ctrl+D for quit):~$ ", talk_user.name, \
               talk_user.host);
        if (fgets(input_msg, sizeof(input_msg), stdin) == NULL) {
            printf("\n\n");
            break;
        }
        if (*input_msg == '\n') {
            continue;
        }

        create_commond(&sendcom, IPMSG_SENDMSG | IPMSG_SENDCHECKOPT, input_msg);
        send_msg(&sendcom, &talk_user.useraddr, sizeof(talk_user.useraddr));
    }

    return 0;
}

int select_files()
{
    printf("Not supported now!\n");
    return 0;
}

int send_files()
{
    return 0;
}

int recv_files(command *com)
{
    printf("%s@%s send a file(s) to you!\n"
           "But this program do not supported now.\n", com->sender_name, \
           com->sender_host);
    return 0;
}

void login()
{
    command com;
    struct sockaddr_in servaddr;

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(IPMSG_DEFAULT_PORT);
    if (inet_pton(AF_INET, allhosts, &servaddr.sin_addr) <= 0) {
        printf("login: inet_pton error.\n");
    }

    create_commond(&com, IPMSG_BR_ENTRY, NULL);
    send_msg(&com, &servaddr, sizeof(servaddr));
    
    return;
}

void logout()
{
    command com;
    struct sockaddr_in servaddr;

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(IPMSG_DEFAULT_PORT);
    if (inet_pton(AF_INET, allhosts, &servaddr.sin_addr) <= 0) {
        printf("logout: inet_pton error.\n");
    }

    create_commond(&com, IPMSG_BR_EXIT, NULL);
    send_msg(&com, &servaddr, sizeof(servaddr));
    
    return;
}
