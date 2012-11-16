#include <stdio.h>
#include "common.h"

int udp_sock;
const char allhosts[] = "255.255.255.255";
int utf8;

int create_sendbuf(char *buf, command *com)
{
    sprintf(buf, "%d:%d:%s:%s:%d:%s", com->version, com->packet_num, \
             com->sender_name, com->sender_host, com->com_num, com->extension);
    return 0;
}

int create_commond(command *com, unsigned int flag)
{
    return 0;
}
