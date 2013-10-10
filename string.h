#ifndef __STRING_H__
#define __STRING_H__
#include <stddef.h>
char *strncpy(char *dest, const char *src, size_t n);
char *strcpy(char *dest, const char *src);
char *strchr(const char *s, int c);
void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *dest, int c, size_t n);
#endif
