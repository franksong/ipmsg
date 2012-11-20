#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include "ipmsg.h"
#include "init.h"
#include "common.h"
#include "send_receive.h"
#include "ipmsg_thread.h"

int interacter()
{
    char input;
    char tmp[100];
    
    printf(HELPINFO);

    while (1) {
        printf("ipmsg~> ");
/*        if (fflush(stdin) != 0) {
            gets(stdin);
        }
*/
        fgets(tmp, 100, stdin);
        input = *tmp;
        switch(input) {
            case 'h':
                printf(HELPINFO);
                break;
            case 'l':
                pthread_mutex_lock(&user_lock);
                list_users();
                pthread_mutex_unlock(&user_lock);
                break;
            case 'r':
                login();
                pthread_mutex_lock(&user_lock);
                list_users();
                pthread_mutex_unlock(&user_lock);
                break;
            case 't':
                talkto_user();
                break;
            case 's':
                select_file();
                break;
            case 'q':
                logout();
                exit(0);
            case 10:
                break;
            default:
                printf("Input error command, please input again.\n");
                break;
        }
    }

    return 0;
}

int processer()
{
    command *current_com;
    unsigned int mode, opt;
    
    while (1) {
        sem_wait(&msg_empty);
        pthread_mutex_lock(&msg_lock);
        current_com = mlist.com_head;
        mlist.com_head = current_com->next;
        pthread_mutex_unlock(&msg_lock);

        if (current_com == NULL) {
            continue;
        }

        mode = GET_MODE(current_com->com_num);
        opt = GET_OPT(current_com->com_num);
        switch (mode) {
            case IPMSG_BR_ENTRY:
                send_recventry(current_com);
                pthread_mutex_lock(&user_lock);
                add_user(current_com);
                pthread_mutex_unlock(&user_lock);
                break;
            case IPMSG_BR_EXIT:
                pthread_mutex_lock(&user_lock);
                del_user(current_com);
                pthread_mutex_unlock(&user_lock);
                break;
            case IPMSG_ANSENTRY:
                pthread_mutex_lock(&user_lock);
                add_user(current_com);
                pthread_mutex_unlock(&user_lock);
                break;
            case IPMSG_SENDMSG:
                if (opt & IPMSG_SENDCHECKOPT) {
                    send_check(current_com);
                }
                if (opt & IPMSG_FILEATTACHOPT) {
                    printf("IN PRO SENDMSG file.\n");
                    ;
                }else
                    putout_msg(current_com);
                break;
            case IPMSG_GETFILEDATA:
                break;
            case IPMSG_GETDIRFILES:
                break;
            case IPMSG_RELEASEFILES:
                break;
            default:
                break;
        }
        free(current_com);
    }

    return 0;
}

int receiver(void *option)
{
    int sock = *((int*)option);
    command *newcom;
    struct sockaddr_in seraddr;
    socklen_t len;
    char recvbuf[MAXLEN];

    while (1) {
        if (recvfrom(sock, recvbuf, sizeof(recvbuf), 0, \
                     (struct sockaddr *)&seraddr, &len) < 0) {
            printf("receiver error: recvfrom() < 0.\n");
            continue;
        }
        printf("\nrecvbuf: %s\n", recvbuf); //debug
        newcom = (command *)malloc(sizeof(command));
        memset(newcom, 0, sizeof(command));
        analysis_recvbuf(recvbuf, newcom);
        memcpy(&newcom->addr, &seraddr, sizeof(newcom->addr));
        newcom->next = NULL;
        
        pthread_mutex_lock(&msg_lock);
        if (mlist.com_head == NULL) {
            mlist.com_head = mlist.com_tail = newcom;
        }else {
            mlist.com_tail->next = newcom;
            mlist.com_tail = newcom;
        }
        sem_post(&msg_empty);
        pthread_mutex_unlock(&msg_lock);
    }

    return 0;
}
