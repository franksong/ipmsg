#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include "ipmsg.h"
#include "common.h"
#include "send_receive.h"

int send_msg(command *option, struct sockaddr *addr, socklen_t len)
{
    char sendbuf[MAXLEN];
    command *com = option;
    
    create_sendbuf(sendbuf, com);
    printf("%s\n", sendbuf); //debug
    sendto(udp_sock, sendbuf, strlen(sendbuf)+1, 0, addr, len);
    
    return 0;
}

int talkto_user()
{
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
    
    init_command(&com);
    com.com_num = IPMSG_BR_ENTRY;
    send_msg(&com, (struct sockaddr *)&servaddr, sizeof(servaddr));
    
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

    init_command(&com);
    com.com_num = IPMSG_BR_EXIT;
    send_msg(&com, (struct sockaddr *)&servaddr, sizeof(servaddr));
    
    return;
}
