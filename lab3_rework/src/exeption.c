#include "exception.h"
#include "uart.h"
#include "timer.h"

int SYNC_64_COUNT = 0;
int IRQ_COUNT     = 0;
int INVALID_COUNT = 0;

void sync_64_router(unsigned long long x0){
    debug_exception(x0, __func__);
    SYNC_64_COUNT++;

    unsigned long long spsr_el1;
	asm volatile("mrs %0, SPSR_EL1\n\t" : "=r" (spsr_el1) :  : "memory");

    unsigned long long elr_el1;
	asm volatile("mrs %0, ELR_EL1\n\t" : "=r" (elr_el1) :  : "memory");

    unsigned long long esr_el1;
	asm volatile("mrs %0, ESR_EL1\n\t" : "=r" (esr_el1) :  : "memory");

    uart_puts("SPSR_EL1: ");
    uart_hex(spsr_el1);
    uart_puts("\n");
    uart_puts("ELR_EL1: ");
    uart_hex(elr_el1);
    uart_puts("\n");
    uart_puts("ESR_EL1: ");
    uart_hex(esr_el1);
    uart_puts("\n");
}

void irq_router(unsigned long long x0) {
    debug_exception(x0, __func__);
    IRQ_COUNT++;
    upTime();
    set_timeout_after(2);
}

void invalid_exception_router(unsigned long long x0) {
    debug_exception(x0, __func__);
    INVALID_COUNT++;

    uart_puts("\nException handler not implemented\n");
    sync_64_router(x0);
}

void enable_interrupt(){
    asm volatile("msr daifclr, 0xf");
}

void disable_interrupt(){
    asm volatile("msr daifset, 0xf");
}

void debug_exception(unsigned long long x0, const char *caller)
{
    // uart_hex(x0);
    // uart_puts(": ");
    // uart_puts_const(caller);
    // uart_puts(" exception\n");
}