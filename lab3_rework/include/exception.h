#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include "gpio.h"

#define CORE0_INTERRUPT_SOURCE ((volatile unsigned int*)(0x40000060))

#define IRQ_BASIC_PENDING	((volatile unsigned int*)(MMIO_BASE+0x0000B200))
#define IRQ_PENDING_1		((volatile unsigned int*)(MMIO_BASE+0x0000B204))
#define IRQ_PENDING_2		((volatile unsigned int*)(MMIO_BASE+0x0000B208))
#define FIQ_CONTROL		    ((volatile unsigned int*)(MMIO_BASE+0x0000B20C))
#define ENABLE_IRQS_1		((volatile unsigned int*)(MMIO_BASE+0x0000B210))
#define ENABLE_IRQS_2		((volatile unsigned int*)(MMIO_BASE+0x0000B214))
#define ENABLE_BASIC_IRQS	((volatile unsigned int*)(MMIO_BASE+0x0000B218))
#define DISABLE_IRQS_1		((volatile unsigned int*)(MMIO_BASE+0x0000B21C))
#define DISABLE_IRQS_2		((volatile unsigned int*)(MMIO_BASE+0x0000B220))
#define DISABLE_BASIC_IRQS	((volatile unsigned int*)(MMIO_BASE+0x0000B224))

#define IRQ_PENDING_1_AUX_INT (1<<29)
#define INTERRUPT_SOURCE_GPU (1<<8)
#define INTERRUPT_SOURCE_CNTPNSIRQ (1<<1)
//#define SYSTEM_TIMER_MATCH_1 (1<<1)
//#define SYSTEM_TIMER_MATCH_3 (1<<3)
//#define UART_INT (1ULL<<57)

void sync_64_router();
void irq_router();
void invalid_exception_router();
void enable_interrupt();
void disable_interrupt();

void debug_exception(unsigned long long x0, const char *caller);

#endif