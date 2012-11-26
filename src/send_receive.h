#ifndef _SEND_RECEIVE_H_
#define _SEND_RECEIVE_H_

#include "common.h"

extern int talkto_user();
extern int send_files();
extern int recv_files(command *com);
extern void login();
extern void logout();

#endif /* _SEND_RECEIVE_H_ */
