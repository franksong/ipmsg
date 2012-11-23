#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>
#include "ipmsg.h"
#include "user_host.h"
#include "common.h"
#include "init.h"
#include "send_receive.h"

//send command message
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

//receive a num from stdin (for select user)
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

//send message to a user
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

//select a file(dir) which want to send
int select_files()
{
    printf("Not supported now!\n");
    return 0;
}

int send_files()
{
    return 0;
}

// receive file(dir) when an user send to you
static int get_data(command *com);
int recv_files(command *com)
{
    printf("Receiving......\n ");
/*    while (1) {
        scanf("%c", &ch);
        if (toupper(ch) == 'Y') {
            break;
        }else if (toupper(ch) == 'N') {
            printf("You refuse to receive the file.\n");
            return 0;
        }else {
            printf("Input error, try again: \n");
            continue;
        }
    }
    fgets(input, MAXLEN, stdin);*/
    pthread_attr_t attr;
    pthread_t tid;
    pthread_attr_init(&attr);
    if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
        printf("pthread_attr_setdetachstate() fail.\n");
        return -1;
    }
    if (pthread_create(&tid, &attr, get_data, com) != 0) {
        printf("pthread_create() fail.\n");
        return -1;
    }
    return 0;
}

//thread: receive file
static int get_data(command *com)
{
    printf("get_date thread start.\n");
    char extension[MAXLEN], sendbuf[MAXLEN];
    char recvbuf[RECVLEN+1];
    char recvdir[NAMELEN*2] = "/home/";
    long offset = 0;
    command sendcom;
    int tcpsock;
    ssize_t recvbytes = 1;
    FILE *recvfile;
    struct sockaddr_in servaddr;
    printf("%s @%s send file: %s in %d.com_num: %d\n", com->sender_name, \
           com->sender_host, com->fileinfo.filename, com->packet_num, \
           com->com_num);

    if ((tcpsock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("get_data thread socket error.\n");
        return -1;
    }
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(IPMSG_DEFAULT_PORT);
    memcpy(&servaddr.sin_addr, &com->addr.sin_addr, sizeof(servaddr.sin_addr));

    snprintf(extension, MAXLEN, "%x:%x:%x", com->packet_num, \
             com->fileinfo.fileID, offset);
    create_commond(&sendcom, IPMSG_GETFILEDATA, extension);
    create_sendbuf(sendbuf, &sendcom);
    if (connect(tcpsock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        printf("get_data thread: connect error.\n");
        return -2;
    }
    strncat(recvdir, sendcom.sender_name, strlen(sendcom.sender_name));
    strcat(recvdir, "/");
    strncat(recvdir, com->fileinfo.filename, strlen(com->fileinfo.filename));
    if ((recvfile = fopen(recvdir, "w+")) == NULL) {
        printf("get_data thread: recvfile create fail.\n");
        return -1;
    }
    if (writen(tcpsock, sendbuf, strlen(sendbuf)+1) < 0) {
        printf("get_data thread: send IPMSG_GETFILEDATA fail.\n");
        return -1;
    }
//    printf("get_data: filesize: %s;%d;%d\n", com->fileinfo.filesize,  \
//           atoi(com->fileinfo.filesize), strlen(com->fileinfo.filesize));
    unsigned int size, recvsize;
    size = hextodec(com->fileinfo.filesize);
    printf("size: %d\n", size);
    recvsize = RECVLEN;
    while (size > 0) {
        if (size < RECVLEN) {
            recvsize = size;
        }
        if ((recvbytes = readn(tcpsock, recvbuf, recvsize)) < 0) {
            printf("get_data thread: recvbyte fail.\n");
            continue;
        }
//        printf("RECVLEN: %d\n", RECVLEN);
//        printf("recvbytes: %d*****\n", recvbytes);
        if (fwrite(recvbuf, sizeof(char), recvbytes, recvfile) < recvbytes) {
            printf("get_data thread: fwrite fail.\n");
            return -1;
        }
        size -= recvbytes;
//        printf("fwrite bytes: %d\n", recvbytes);
    }
    
    fclose(recvfile);
    close(tcpsock);
    printf("receive file finished.\n");
    free(com);
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
