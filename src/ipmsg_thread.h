#ifndef _IPMSG_THREAD_H_
#define _IPMSG_THREAD_H_

#define HELPINFO \
    "*       IPMSG COMMAND       *\n"\
    "* h: help.                  *\n"\
    "* l: listusers.             *\n"\
    "* r: reflush users.         *\n"\
    "* t: talk to other users.   *\n"\
    "* s: sendfile to user.      *\n"\
    "* q: quit.                  *\n"

extern int interacter();
extern int processer();
extern int receiver(void *option);

#endif /* _IPMSG_THREAD_H_ */
