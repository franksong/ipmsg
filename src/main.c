#include <pthread.h>
#include "init.h"
#include "ipmsg_thread.h"
#include "common.h"

int main(int argc, char *argv[])
{
    pthread_t inter, proc, recv;

    init_ipmsg();

    pthread_create(&recv, NULL, receiver, &udp_sock);
    pthread_create(&proc, NULL, processer, NULL);
    pthread_create(&inter, NULL, interacter, NULL);

    pthread_join(recv, NULL);
    pthread_join(proc, NULL);
    pthread_join(inter, NULL);
    return 0;
}
