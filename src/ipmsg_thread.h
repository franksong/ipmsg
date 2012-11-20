#ifndef _IPMSG_THREAD_H_
#define _IPMSG_THREAD_H_

#define HELPINFO \
    "\t*****************************\n"\
    "\t*       IPMSG COMMAND       *\n"\
    "\t* h: help.                  *\n"\
    "\t* l: listusers.             *\n"\
    "\t* r: reflush users.         *\n"\
    "\t* t: talk to other users.   *\n"\
    "\t* s: sendfile to user.      *\n"\
    "\t* q: quit.                  *\n"\
    "\t*****************************\n"

extern int interacter();
extern int processer();
extern int receiver(void *option);

#endif /* _IPMSG_THREAD_H_ */
