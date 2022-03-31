#ifndef _LIB_H
#define _LIB_H

int strcmp(char *s1, char *s2);
int strncmp (const char *s1, const char *s2, unsigned long long n);
int len(char *s);
char* strchr (register const char *s, int c);
int atoi(char *s);
unsigned long pass_x0(unsigned long x0);
char* strcpy (char *dest, const char *src);
char* memcpy (void *dest, const void *src, unsigned long long len);
#endif