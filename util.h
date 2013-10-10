#ifndef __UTIL_H__
#define __UTIL_H__
#define itoa(val) itoa_base(val, 10)
#define htoa(val) itoa_base(val, 16)

char* itoa_base(int val, int base);
#endif
