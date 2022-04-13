#ifndef _LIB_H
#define _LIB_H

int strcmp(char *s1, char *s2);
int len(char *s);
int atoi(char *s);
char* memcpy (void *dest, void *src, unsigned long long len);
char* strcpy (char *dest, char *src);
unsigned long pass_x0(unsigned long x0);
int strncmp (char *s1, char *s2, unsigned long long n);
char* strchr (register char *s, int c);
#endif