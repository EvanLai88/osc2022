#include "exceptionHandler.h"
#include "uart.h"

void sync_64_router(unsigned long long x0){
    uart_puts("sync_64_router exception\n");
    unsigned long long spsr_el1;
	asm volatile("mrs %0, SPSR_EL1\n\t" : "=r" (spsr_el1) :  : "memory");

    unsigned long long elr_el1;
	asm volatile("mrs %0, ELR_EL1\n\t" : "=r" (elr_el1) :  : "memory");

    unsigned long long esr_el1;
	asm volatile("mrs %0, ESR_EL1\n\t" : "=r" (esr_el1) :  : "memory");

    uart_puts("Exception: sync_el0_64_router\n");
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

void invalid_exception_router(unsigned long long x0) {
    uart_hex(x0);
    uart_puts("\nException handler not implemented\n");
    sync_64_router(x0);
}

void irq_router(unsigned long long x0) {
    uart_puts("irq_router exception\n");
}

void enable_interrupt(){
    asm volatile("msr daifclr, 0xf");
}

void disable_interrupt(){
    asm volatile("msr daifset, 0xf");
}