#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ipmsg.h"
#include "user_host.h"
#include "init.h"
#include "common.h"
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

int talkto_user()
{
    char input_msg[MAXLEN];
    int who, count;
    user talk_user;
    command sendcom;
    
    pthread_mutex_lock(&user_lock);
    count = list_users();
    pthread_mutex_unlock(&user_lock);
    printf("Input the Num who you want to talk(0 for cancel): ");
    while (1) {
        scanf("%d", &who);
        if (who == 0) {
            printf("\n");
            return 0;
        }
        if (who < 1 || who > count) {
            printf("ERROR! no this user, input again: ");
            continue;
        }
        break;
    }
    getchar();
    pthread_mutex_lock(&user_lock);
    memcpy(&talk_user, ulist.user_head+(who-1), sizeof(struct user));
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

int select_file()
{
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
