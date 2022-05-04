#ifndef _COMPILER_H
#define _COMPILER_H

#define uint32_t unsigned int
#define uint64_t unsigned long long

#define likely(x)          __builtin_expect(!!(x), 1)
#define unlikely(x)        __builtin_expect(!!(x), 0)

#define STR(x) #x
#define XSTR(s) STR(s)

#endif