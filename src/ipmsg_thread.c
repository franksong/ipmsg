#include <stdio.h>
#include "ipmsg.h"
#include "send_receive.h"
#include "ipmsg_thread.h"

int interacter()
{
    char input;
    char tmp[100];
    
    printf(HELPINFO);

    while (1) {
        printf("ipmsg~> ");
        fgets(tmp, 100, stdin);
        input = *tmp;
        switch(input) {
            case 'h':
                printf(HELPINFO);
                break;
            case 'l':
                list_users();
                break;
            case 'r':
                login();
                list_users();
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
            default:
                printf("Input error command, please input again.\n");
                break;
        }
    }

    return 0;
}

int processer()
{
    return 0;
}

int receiver(void *option)
{
    return 0;
}
