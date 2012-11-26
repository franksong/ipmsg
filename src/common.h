#ifndef _COMMON_H_
#define _COMMON_H_

#include <semaphore.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netinet/in.h>
#define NAMELEN 100
#define HOSTLEN 100
#define MAXLEN  1024
#define RECVLEN 1500
#define SENDLEN 1500
#define LISTENQ 1024

struct file_info {
    unsigned int  fileID;
//    char fileID[NAMELEN];
    char filename[NAMELEN];
    char filesize[NAMELEN];
    char filemtime[NAMELEN];
    unsigned int fileattr;
};

typedef struct command {
    unsigned int version;
    unsigned int packet_num;
    char sender_name[NAMELEN];
    char sender_host[HOSTLEN];
    unsigned int com_num;
    char extension[MAXLEN];
    struct sockaddr_in addr;
    struct file_info fileinfo;
    struct commond *next;
} command;

typedef struct msg_list {
    command *com_head;
    command *com_tail;
} msg_list;

extern msg_list mlist;
extern int udp_sock;
extern const char allhosts[];
extern int utf8;

extern int create_sendbuf(char *sendbuf, command *com);
extern int analysis_recvbuf(char *recvbuf, command *com);

extern int send_check(command *com);
extern int putout_msg(command *com);
extern int send_recventry(command *com);
extern ssize_t readn(int fd, void *vptr, size_t n);
extern ssize_t writen(int fd, const void *vptr, size_t n);
extern unsigned int hextodec(const char *hex);

#endif /* _COMMON_H_ */
