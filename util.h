#ifndef __UTIL_H__
#define __UTIL_H__
#define MAX_ITOA_CHARS (32)

char *itoa(int val, char *itoa_buf);
char *htoa(int val, char *htoa_buf);
char* addrtoa(long int addr, char *addr_buf);
#endif
