#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include "gpio.h"

#define CORE0_INTERRUPT_SOURCE ((volatile unsigned int*)(0x40000060))

void sync_64_router();
void irq_router();
void invalid_exception_router();
void enable_interrupt();
void disable_interrupt();

void debug_exception(unsigned long long x0, const char *caller);

#endif