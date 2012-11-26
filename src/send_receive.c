#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
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
    int buflen, tmp;

    create_sendbuf(sendbuf, com);
    buflen = strlen(sendbuf);
    if (com->com_num & IPMSG_FILEATTACHOPT) {
        printf("send_msg: send file msg.\n");
        tmp = strlen(sendbuf+buflen+1);
        if (sendto(udp_sock, sendbuf, buflen+tmp+2, 0, (struct sockaddr *)addr, \
                   len) < 0) {
            printf("send_msg: sendfilecommand: sendto error.\n");
            return -1;
        }
    }else if (sendto(udp_sock, sendbuf, strlen(sendbuf)+1, 0, \
                     (struct sockaddr *)addr, len) < 0) {
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

    printf("Input num of user who you want to talk/send(cancel: 0.): ");
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

//select a user to talk or sendfile
static int select_user(user *user_ptr)
{
    user *ptruser;
    int count, who;
    
    pthread_mutex_lock(&user_lock);
    count = list_users();
    pthread_mutex_unlock(&user_lock);

    who = input_num(1, count, 1);
    
    if (who == 0) {
        return -1;
    }
    pthread_mutex_lock(&user_lock);
    ptruser = ulist.user_head;
    who--;
    while (who) {
        ptruser = ptruser->next;
        who--;
    }
    memcpy(user_ptr, ptruser, sizeof(struct user));
    pthread_mutex_unlock(&user_lock);
    
    return 0;
}

//send message to a user
int talkto_user()
{
    char input_msg[MAXLEN];
    user talk_user;
    command sendcom;
    
    if (select_user(&talk_user) == -1) {
        printf("\n");
        return 0;
    }
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
static int send_data(command *com);
int send_files()
{
    printf("send file!!!\n");
    user senduser;
    command *sendcom;
    struct stat filebuf;
    char name[NAMELEN];
    char msg[MAXLEN] = "Frank";
    if (select_user(&senduser) == -1) {
        printf("\n");
        return 0;
    }
    printf("\nInput send filename: ");
    while (1) {
        if (fgets(name, NAMELEN, stdin) == NULL) {
            return 0;
        }
        printf("strlenfilename: %d\n", strlen(name));
        name[strlen(name)-1] = '\0';
        printf("strlenfilename: %d\n", strlen(name));
        if (lstat(name, &filebuf) < 0) {
            printf("senffilename: %s\n", name);
            printf("The file isn't existed or no permissions. Try again:\n");
            continue;
        }
        if (S_ISDIR(filebuf.st_mode)) {
            printf("Not support dir.\n");
            continue;
        }
        break;
    }
    sendcom = (command *)malloc(sizeof(struct command));
    create_commond(sendcom, IPMSG_SENDMSG | IPMSG_FILEATTACHOPT, msg);
    memcpy(sendcom->fileinfo.filename, name, sizeof(name));
    sendcom->fileinfo.fileID = (unsigned int)time(&filebuf.st_atime);
    snprintf(sendcom->fileinfo.filesize, NAMELEN, "%x", filebuf.st_size);
    snprintf(sendcom->fileinfo.filemtime, NAMELEN, "%x", filebuf.st_mtime);
    sendcom->fileinfo.fileattr = 1;
    memcpy(&sendcom->addr, &senduser.useraddr, sizeof(senduser.useraddr));

    pthread_attr_t attr;
    pthread_t tid;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&tid, &attr, send_data, sendcom);
    
    return 0;
}

static unsigned int transmit_data(command *com, int fd)
{
    FILE *sendfile;
    unsigned int filesize, readbytes;
    char sendbuf[SENDLEN];
    
    filesize = hextodec(com->fileinfo.filesize);
    if ((sendfile = fopen(com->fileinfo.filename, "r+")) == NULL) {
        printf("tarnsmit_data: fopen error.\n");
        return -1;
    }
    while (filesize > 0) {
        readbytes = SENDLEN > filesize ? filesize : SENDLEN;
        fread(sendbuf, sizeof(char), readbytes, sendfile);
        writen(fd, sendbuf, sizeof(sendbuf));
    }

    return 0;
}
static int send_data(command *com)
{
    printf("send_data thread create finish.\n");
    int tcpsock, connfd;
    unsigned int ipmsg_com, pack_id, file_id;
    struct sockaddr_in servaddr;
    char recvbuf[MAXLEN];
    char *tmp, *ptr;
    
    if ((tcpsock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("send_data: socket error.\n");
        return -1;
    }
    printf("send_data: socket finished.\n");//debug
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(IPMSG_DEFAULT_PORT);
    if (bind(tcpsock, (struct servaddr *)&servaddr, sizeof(servaddr)) != 0) {
        printf("send_data: bind error.\n");
        return -1;
    }
    printf("send_data: bind finished.\n");//debug
    if (listen(tcpsock, LISTENQ) != 0) {
        printf("send_data: listen error.\n");
        return -1;
    }
    printf("send_data: listen finished.\n");//debug
    send_msg(com, &com->addr, sizeof(com->addr));
    while (1) {
        connfd = accept(tcpsock, (struct servaddr *)NULL, NULL);
        if (connfd < 0) {
            printf("send_data: accept error.\n");
            continue;
        }
        printf("send_data: accept finished.\n");//debug
        transmit_data(com, connfd);
        break;
/*        readn(connfd, recvbuf, sizeof(recvbuf));
        printf("send_data: recvbuf: \n", recvbuf);
        if (recvbuf == NULL) {
            break;
        }

        tmp = recvbuf;
        int i;
        for (i = 0; i < 4; ++i) {
            tmp = strstr(tmp, ":");
            tmp++;
        }
        ptr = tmp;
        tmp = strstr(tmp, ":");
        *tmp++ = '\0';
        ipmsg_com = atoi(ptr);
        ptr = tmp;
        tmp = strstr(tmp, ":");
        *tmp++ = '\0';
        pack_id = hextodec(ptr);
        ptr = tmp;
        tmp = strstr(tmp, ":");
        *tmp = '\0';
        file_id = hextodec(ptr);
        if (ipmsg_com & IPMSG_GETFILEDATA) {
            if (pack_id != com->packet_num) {
                close(connfd);
                continue;
            }else if (file_id != com->fileinfo.fileID) {
                close(connfd);
                continue;
            }else {
                printf("tarnsmit_data:\n");
                transmit_data(com, connfd);
                break;
            }
        }else {
            close(connfd);
            continue;
        }*/
    }
    close(connfd);
    free(com);
    printf("send file finished.\n");
    return 0;
}

// receive file(dir) when an user send to you
static int recv_data(command *com);
int recv_files(command *com)
{
    printf("Receiving......\n ");
    pthread_attr_t attr;
    pthread_t tid;
    pthread_attr_init(&attr);
    if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
        printf("pthread_attr_setdetachstate() fail.\n");
        return -1;
    }
    if (pthread_create(&tid, &attr, recv_data, com) != 0) {
        printf("pthread_create() fail.\n");
        return -1;
    }
    return 0;
}

//thread: receive file
static int recv_data(command *com)
{
    printf("recv_date thread start.\n");
    char extension[MAXLEN], sendbuf[MAXLEN];
    char recvbuf[RECVLEN+1];
    char recvdir[NAMELEN*2] = "/home/";
    int offset = 0;
    command sendcom;
    int tcpsock;
    ssize_t recvbytes = 1;
    FILE *recvfile;
    struct sockaddr_in servaddr;
    printf("%s@%s send file: %s in %d.com_num: %d\n", com->sender_name, \
           com->sender_host, com->fileinfo.filename, com->packet_num, \
           com->com_num);

    if ((tcpsock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("recv_data thread socket error.\n");
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
        printf("recv_data thread: connect error.\n");
        return -2;
    }
//    strncat(recvdir, sendcom.sender_name, strlen(sendcom.sender_name));
//    strcat(recvdir, "/");
    strncpy(recvdir, com->fileinfo.filename, strlen(com->fileinfo.filename));
    if ((recvfile = fopen(recvdir, "w+")) == NULL) {
        printf("recv_data thread: recvfile create fail.\n");
        return -1;
    }
    if (writen(tcpsock, sendbuf, strlen(sendbuf)+1) < 0) {
        printf("recv_data thread: send IPMSG_GETFILEDATA fail.\n");
        return -1;
    }
    unsigned int size, recvsize;
    size = hextodec(com->fileinfo.filesize);
    printf("size: %d\n", size);
    recvsize = RECVLEN;
    while (size > 0) {
        if (size < RECVLEN) {
            recvsize = size;
        }
        if ((recvbytes = readn(tcpsock, recvbuf, recvsize)) < 0) {
            printf("recv_data thread: recvbyte fail.\n");
            continue;
        }
        if (fwrite(recvbuf, sizeof(char), recvbytes, recvfile) < recvbytes) {
            printf("recv_data thread: fwrite fail.\n");
            return -1;
        }
        size -= recvbytes;
    }
    
    fclose(recvfile);
    close(tcpsock);
    printf("recv file finished!!!\n");
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
