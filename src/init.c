#include <sys/utsname.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pwd.h>
#include <string.h>
#include "ipmsg.h"
#include "init.h"

static char user_name[NAMELEN];
static char host_name[HOSTLEN];

static void get_user_info();
static int init_sock();

void init_ipmsg()
{
    get_user_info();
/*    utf8 = 0;
    if (setlocale(LC_CTYPE, "")) {
        if (!strcmp(nl_langinfo(CODESET), "UTF-8")) {
            utf8 = 1;
        }

        } */

    init_sock();
    login();
//    printf("init_ipmsg.\n"); // debug
}

int init_command(command *com)
{
    com->version = 1;
    com->packet_num = (unsigned int)time(NULL);
    strncpy(com->sender_name, user_name, sizeof(com->sender_name));
    strncpy(com->sender_host, host_name, sizeof(com->sender_host));
    
    return 0;
}

static void get_user_info()
{
    struct utsname name;
    struct passwd *pwd;
    
    uname(&name);
    pwd = getpwuid(getuid());
    strncpy(user_name, pwd->pw_name, sizeof(user_name));
    strncpy(host_name, name.nodename, sizeof(host_name));
    
    return;
}

static int init_sock()
{
    struct sockaddr_in servaddr;
    const int on = 1;
    
    udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock < 0) {
        printf("init->init_sock: socket fail.\n");
        exit(1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(IPMSG_DEFAULT_PORT);
    if (setsockopt(udp_sock, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) < 0) {
        printf("init_sock: udp_sock set option error.\n");
        exit(1);
    }

    if (bind(udp_sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
        printf("init->init_sock: bind fail.\n");
        exit(1);
    }

    return 0;
}
