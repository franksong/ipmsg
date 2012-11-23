#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>
#include "ipmsg.h"
#include "common.h"

int udp_sock;
const char allhosts[] = "255.255.255.255"; //broadcase address
int utf8;
msg_list mlist;

//create the buf will be send from command
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

// analysis the buf received to command
static int analysis_fileinfo(struct file_info *fileinfo, char *buf);
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
                if (GET_OPT(com->com_num) & IPMSG_FILEATTACHOPT)
//                    printf("in analysis before fileinfo.\n");//debug
                    analysis_fileinfo(&(com->fileinfo), com->extension);
                    
                break;
            default:
                break;
        }
        buf = tmp;
        index++;
    }

    return 0;
}

//create the commond will be send
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

//send confirm command (when receive IPMSG_SENDMSG|IPMSG_CHECKOPT)
int send_check(command *com)
{
    command sendcom;
    
    create_commond(&sendcom, IPMSG_RECVMSG, NULL);
    sprintf(sendcom.extension, "%d", com->packet_num);
    send_msg(&sendcom, &(com->addr), sizeof(com->addr));
    
    return 0;
}

//put out the message received
int putout_msg(command *com)
{
    printf("Massage from %s@%s:~# \n", com->sender_name, com->sender_host);
    puts(com->extension);
    printf("\n");
    return 0;
}

//send IPMSG_ANSENTRY command(when recv IPMSG_BR_ENTRY)
int send_recventry(command *com)
{
    command sendcom;

    create_commond(&sendcom, IPMSG_ANSENTRY, NULL);
    snprintf(sendcom.extension, "%s", sendcom.sender_name);
    send_msg(&sendcom, &(com->addr), sizeof(com->addr));
    return 0;
}

static int analysis_fileinfo(struct file_info *fileinfo, char *buf)
{
    int index = 0, count = 0;
    size_t min;
    char *pos = NULL, *start;
    int i = 0;
    while (i < MAXLEN) {
        putchar(buf[i]);
        i++;
    }
    buf = buf+strlen(buf)+1;
    while (index < 5) {
        if (buf == NULL) {
            return 0;
        }
        start = strstr(buf, ":");
        if (start == NULL) {
            return 0;
        }

        pos = start;
        count = 1;
        while (*(pos+1) == ':') {
            count++;
            pos++;
        }
        if (count % 2 == 0) {
            start = pos+1;
            continue;
        }
        switch (index) {
            case 0:
                *start = '\0';
                fileinfo->fileID = atoi(buf);
                break;
            case 1:
                min = start - buf;
                memcpy(fileinfo->filename, buf, min);
                fileinfo->filename[min] = '\0';
                break;
            case 2:
                min = start - buf;
                memcpy(fileinfo->filesize, buf, min);
                fileinfo->filesize[min] = '\0';
                break;
            case 3:
                min = start - buf;
                memcpy(fileinfo->filemtime, buf, min);
                fileinfo->filemtime[min] = '\0';
                break;
            case 4:
                sscanf(buf, "%x", &fileinfo->fileattr);
                break;
            default:
                break;
        }
        index++;
        buf = start+1;
    }

    return 0;
}

ssize_t readn(int fd, void *vptr, size_t n)
{
    size_t nleft;
    ssize_t nread;
    char *ptr;

    ptr = vptr;
    nleft = n;
//    printf("in nread n = %d\n", n);
    while (nleft > 0) {
        if ((nread = read(fd, ptr, nleft)) < 0) {
            if (errno == EINTR) {
                continue;
            }else{
                printf("In readn readbyte: %d.return -1\n", nread);
                return -1;
            }
        }else if (nread == 0) {
//            return 0;
            break;
        }
        nleft -= nread;
        ptr += nread;
//        printf("in nread while nleft = %d\n", nleft);
    }
//    printf("In readn readbyte: %d\n", nread);

    return (n-nleft);
}

ssize_t writen(int fd, const void *vptr, size_t n)
{
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ((nwritten = write(fd, ptr, nleft)) <= 0) {
            if (nwritten < 0 && errno == EINTR) {
                nwritten = 0;
            }else
                return -1;
        }
        nleft -= nwritten;
        ptr += nwritten;
//        printf("writen: nleft = %d, nwritten = %d\n", nleft, nwritten);//debug
    }
    return n;
}

//hexadecimal string to decimal int
static unsigned int my_pow(int x, int y)
{
    int sum = 1;
    if (y == 0) {
        return 1;
    }
    while (y) {
        sum *= x;
        y--;
    }
    return sum;
}
unsigned int hextodec(const char *hex)
{
    int size, sum = 0;
    char *ptr;

    ptr = hex;
    size = strlen(hex);
    size--;
    while (*ptr != NULL) {
        switch (toupper(*ptr)) {
            case '0':
                sum += 0*my_pow(16, size);
                break;
            case '1':
                sum += 1*my_pow(16, size);
                break;
            case '2':
                sum += 2*my_pow(16, size);
                break;
            case '3':
                sum += 3*my_pow(16, size);
                break;
            case '4':
                sum += 4*my_pow(16, size);
                break;
            case '5':
                sum += 5*my_pow(16, size);
                break;
            case '6':
                sum += 6*my_pow(16, size);
                break;
            case '7':
                sum += 7*my_pow(16, size);
                break;
            case '8':
                sum += 8*my_pow(16, size);
                break;
            case '9':
                sum += 9*my_pow(16, size);
                break;
            case 'A':
                sum += 10*my_pow(16, size);
                break;
            case 'B':
                sum += 11*my_pow(16, size);
                break;
            case 'C':
                sum += 12*my_pow(16, size);
                break;
            case 'D':
                sum += 13*my_pow(16, size);
                break;
            case 'E':
                sum += 14*my_pow(16, size);
                break;
            case 'F':
                sum += 15*my_pow(16, size);
                break;
            default:
                return -1;
                break;
        }
        ptr++;
        size--;
    }

    return sum;
}
