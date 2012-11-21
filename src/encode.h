#ifndef _ENCODE_H_
#define _ENCODE_H_

#include <unistd.h>

extern int g2u(char *inbuf, size_t inlen, char *outbuf, size_t outlen);
extern int u2g(char *inbuf, int inlen, char *outbuf, int outlen);

#endif /* _ENCODE_H_ */
