#include "lib.h"

int strcmp(char *s1, char *s2) {
    
    while ( *s1 != '\0' && *s2 != '\0' ) {
        if ( *s1 != *s2 ) {
            return (int) (*s1 - *s2);
        }
        s1++;
        s2++;
    }
    if ( *s1 == *s2 ) {
        return 0;
    } 

    return (int) (*s1 - *s2);

}

int len(char *s) {
    int digits = 0;
    while(*s++) {
        digits++;
    }

    return digits;
}

int atoi(char *s) {
    char *str;
    int digits, num = 0, tmp;
    
    str = s;
    digits = len(str);
    
    for(int i=0; i<digits; i++) {
        tmp = *str++ - '0';
        for (int j = 0; j<digits-i-1; j++) {
            tmp *= 10;
        }
        num += tmp;
    }
    return num;
}

char* memcpy (void *dest, void *src, unsigned long long len)
{
  char *d = dest;
  const char *s = src;
  while (len--)
    *d++ = *s++;
  return dest;
}

char* strcpy (char *dest, char *src)
{
  return memcpy (dest, src, len (src) + 1);
}

unsigned long pass_x0(unsigned long x0) {
    return x0;
}

int strncmp (char *s1, char *s2, unsigned long long n)
{
  unsigned char c1 = '\0';
  unsigned char c2 = '\0';
  if (n >= 4)
    {
      unsigned int n4 = n >> 2;
      do
        {
          c1 = (unsigned char) *s1++;
          c2 = (unsigned char) *s2++;
          if (c1 == '\0' || c1 != c2)
            return c1 - c2;
          c1 = (unsigned char) *s1++;
          c2 = (unsigned char) *s2++;
          if (c1 == '\0' || c1 != c2)
            return c1 - c2;
          c1 = (unsigned char) *s1++;
          c2 = (unsigned char) *s2++;
          if (c1 == '\0' || c1 != c2)
            return c1 - c2;
          c1 = (unsigned char) *s1++;
          c2 = (unsigned char) *s2++;
          if (c1 == '\0' || c1 != c2)
            return c1 - c2;
        } while (--n4 > 0);
      n &= 3;
    }
  while (n > 0)
    {
      c1 = (unsigned char) *s1++;
      c2 = (unsigned char) *s2++;
      if (c1 == '\0' || c1 != c2)
        return c1 - c2;
      n--;
    }
  return c1 - c2;
}

char* strchr (register char *s, int c)
{
  do {
    if (*s == c)
      {
        return (char*)s;
      }
  } while (*s++);
  return (0);
}