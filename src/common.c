#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "ipmsg.h"
#include "common.h"

int udp_sock;
const char allhosts[] = "255.255.255.255";
int utf8;
msg_list mlist;

int create_sendbuf(char *sendbuf, command *com)
{
    char tmp[MAXLEN];
    snprintf(tmp, MAXLEN-1, "%d:%d:%s:%s:%d:%s",
            com->version,
            com->packet_num,
            com->sender_name,
            com->sender_host,
            com->com_num,
            com->extension);
    if (utf8) {
        u2g(tmp, sizeof(tmp), sendbuf, MAXLEN-1);
    }else
        strncpy(sendbuf, tmp, MAXLEN-1);

    return 0;
}

/*
static char *search_char(char *buf, char c)
{
    char *tmp = buf;
    while (*tmp != '\0') {
        if (*tmp == c) {
            *tmp++ = '\0';
            return tmp;
        }
        tmp++;
    }
    return NULL;
}
*/

int analysis_recvbuf(char *recvbuf, command *com)
{
    char *buf, *tmp;
    int index = 0;
    char code[MAXLEN];

    if (utf8) {
        g2u(recvbuf, MAXLEN, code, sizeof(code));
        buf = code;
    }else
        buf = recvbuf;

    while (index < 6) {
        tmp = strstr(buf, ":");
        if (tmp != NULL) {
            *tmp++ = '\0';
        }
        switch (index) {
            case 0:
                com->version = atoi(buf);
                break;
            case 1:
                com->packet_num = atoi(buf);
                break;
            case 2:
                memcpy(com->sender_name, buf, sizeof(com->sender_name));
                break;
            case 3:
                memcpy(com->sender_host, buf, sizeof(com->sender_host));
                break;
            case 4:
                com->com_num = atoi(buf);
                break;
            case 5:
                memcpy(com->extension, buf, sizeof(com->extension));
                break;
            default:
                break;
        }
        buf = tmp;
        index++;
    }

    return 0;
}

int create_commond(command *com, unsigned int flag, char *extension)
{
    init_command(com);
    com->com_num = flag;
    if (extension == NULL) {
        com->extension[0] = '\0';
    }else
        memcpy(com->extension, extension, sizeof(com->extension));
    return 0;
}

int send_check(command *com)
{
    command sendcom;
    
    create_commond(&sendcom, IPMSG_RECVMSG, NULL);
    sprintf(sendcom.extension, "%d", com->packet_num);
    send_msg(&sendcom, &(com->addr), sizeof(com->addr));
    
    return 0;
}

int putout_msg(command *com)
{
    printf("Massage from %s@%s:~# \n", com->sender_name, com->sender_host);
    puts(com->extension);
    printf("\n");
    return 0;
}

int send_recventry(command *com)
{
    command sendcom;

    create_commond(&sendcom, IPMSG_ANSENTRY, NULL);
    snprintf(sendcom.extension, "%s", sendcom.sender_name);
    send_msg(&sendcom, &(com->addr), sizeof(com->addr));
    return 0;
}
