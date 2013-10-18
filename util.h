#ifndef __UTIL_H__
#define __UTIL_H__
#define MAX_ITOA_CHARS (32)

/* Warning!! Last atoi buf element needs to be set to \0 */
/*           for itoa(), htoa() and addrtoa()            */
/* Example: itoa_buf[MAX_ITOA_CHARS - 1] = 0;            */
/*          itoa_buf = itoa(100, itoa_buf)               */
char *itoa(int val, char *itoa_buf);
char *htoa(int val, char *htoa_buf);
char* addrtoa(long int addr, char *addr_buf);
#endif
