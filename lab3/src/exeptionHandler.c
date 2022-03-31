#include "exceptionHandler.h"
#include "uart.h"
#include "timer.h"
#include "list.h"

void sync_64_router(unsigned long long x0){
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
    uart_putln("Exception handler not implemented");
    sync_64_router(x0);
}

void irq_router(unsigned long long x0) {
    // uart_puts("\nCORE0_INTERRUPT_SOURCE: ");
    // uart_hex(*CORE0_INTERRUPT_SOURCE);
    // uart_puts("\n");

    // uart_puts("CORE0_TIMER_IRQ_CTRL: ");
    // uart_hex(*((volatile unsigned int*)CORE0_TIMER_IRQ_CTRL));
    // uart_puts("\n");

    if(*IRQ_PENDING_1 & IRQ_PENDING_1_AUX_INT && *CORE0_INTERRUPT_SOURCE & INTERRUPT_SOURCE_GPU)
    {
        if (*AUX_MU_IIR & (0b01 << 1)) //can write
        {
            disable_mini_uart_w_interrupt(); // lab 3 : advanced 2 -> mask device line (enable by handler)
            uart_interrupt_w_handler();
            // run_preemptive_tasks();
        }
        else if (*AUX_MU_IIR & (0b10 << 1)) // can read
        {
            disable_mini_uart_r_interrupt(); // lab 3 : advanced 2 -> mask device line (enable by handler)
            uart_interrupt_r_handler();
            // run_preemptive_tasks();
        }
        else
        {
            uart_puts("uart handler error\n");
        }
    }else if(*CORE0_INTERRUPT_SOURCE & INTERRUPT_SOURCE_CNTPNSIRQ)
    {
        // core_timer_disable();
        core_timer_handler();
        // core_timer_enable();
    }
}

void enable_interrupt(){
    asm volatile("msr daifclr, 0xf");
}

void disable_interrupt(){
    asm volatile("msr daifset, 0xf");
}