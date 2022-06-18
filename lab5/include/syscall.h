#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "compiler.h"
#include "exception.h"

// int getpid(trapframe_t *tpf);
// size_t uart_read(trapframe_t *tpf,char buf[], size_t size);
// size_t uart_write(trapframe_t *tpf, const char buf[], size_t size);
// int exec(trapframe_t *tpf,const char *name, char *const argv[]);
// int fork(trapframe_t *tpf);
// void exit(trapframe_t *tpf, int status);
// int sys_mbox_call(trapframe_t *tpf, unsigned char ch, unsigned int *mbox);
// void kill(trapframe_t *tpf, int pid);
// void signal_register(int signal, void (*handler)());
// void signal_kill(int pid, int signal);
// void sigreturn(trapframe_t *tpf);


int getpid(trapframe_t *tpf);
size_t uartread(trapframe_t *tpf,char buf[], size_t size);
size_t uartwrite(trapframe_t *tpf,const char buf[], size_t size);
int exec(trapframe_t *tpf,const char *name, char *const argv[]);
int fork(trapframe_t *tpf);
void exit(trapframe_t *tpf,int status);
int syscall_mbox_call(trapframe_t *tpf, unsigned char ch, unsigned int *mbox);
void kill(trapframe_t *tpf,int pid);
void signal_register(int signal, void (*handler)());
void signal_kill(int pid, int signal);
void sigreturn(trapframe_t *tpf);

#endif