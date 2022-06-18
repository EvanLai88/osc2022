#ifndef _SIGNAL_H
#define _SIGNAL_H

#include "syscall.h"
#include "sched.h"
#include "malloc.h"

void signal_default_handler();
void check_signal(trapframe_t *tpf);
void run_signal(trapframe_t* tpf,int signal);
void signal_handler_wrapper();

#endif