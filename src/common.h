#ifndef _COMMON_H_
#define _COMMON_H_

#define NAMELEN 100
#define HOSTLEN 100
#define MAXLEN  1024

typedef struct command {
    unsigned int version;
    unsigned int packet_num;
    char sender_name[NAMELEN];
    char sender_host[HOSTLEN];
    unsigned int com_num;
    char extension[MAXLEN];
    struct commond *next;
} command;

typedef struct msg_list {
    command *com_head;
    command *com_tail;
} msg_list;

extern int udp_sock;
extern const char allhosts[];
extern int utf8;

extern int create_sendbuf(char *buf, command *com);

#endif /* _COMMON_H_ */
