#ifndef _INIT_H_
#define _INIT_H_

#include <semaphore.h>
#include "common.h"

extern pthread_mutex_t msg_lock;
extern pthread_mutex_t user_lock;
extern sem_t msg_empty;

extern void init_ipmsg();
extern int init_command(command *com);

#endif /* _INIT_H_ */
