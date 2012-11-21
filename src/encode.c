#include <iconv.h>
#include <string.h>

int code_convert(char *to_charset, char *from_charset,
                 char *inbuf, int inlen, char *outbuf, int outlen)
{
  
  iconv_t cd;
  int flag;
  char **pin = &inbuf;
  char **pout = &outbuf;

  flag = 0;

  if ((cd=iconv_open(to_charset, from_charset))==(iconv_t) -1)
    flag = -1;
  
  bzero(outbuf,outlen);
  
  if (iconv(cd,pin, &inlen, pout, &outlen)==(size_t) -1)
    flag = -1;
  
  iconv_close(cd);
  return flag;
}

//gb2312 to utf-8
int g2u(char *inbuf, int inlen, char *outbuf, int outlen)
{
  return code_convert("utf-8", "gb2312", inbuf, inlen, outbuf, outlen);
}

//utf-8 to gb2312
int u2g(char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
  return code_convert("gb2312", "utf-8", inbuf, inlen, outbuf, outlen);
}
