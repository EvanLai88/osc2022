#include "exception.h"
#include "uart.h"
#include "timer.h"
#include "task.h"

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

    disable_mini_uart_interrupt();
    uart_puts("SPSR_EL1: ");
    uart_hex(spsr_el1);
    uart_puts("\n");
    uart_puts("ELR_EL1: ");
    uart_hex(elr_el1);
    uart_puts("\n");
    uart_puts("ESR_EL1: ");
    uart_hex(esr_el1);
    uart_puts("\n");
    enable_mini_uart_interrupt();
}

void irq_router(unsigned long long x0) {
    debug_exception(x0, __func__);
    IRQ_COUNT++;
    unsigned long long tmp = *IRQ_PEND1 & (1<<29);
    if (tmp)
    {
        // disable_mini_uart_interrupt();
        // uart_putln("IO exception");
        // uart_puts("\tIRQ_PEND1: ");
        // uart_hex(tmp);
        // uart_puts("\n");
        // enable_mini_uart_interrupt();

        if (*CORE0_INTERRUPT_SOURCE & (1<<8)) // from aux && from GPU0 -> uart exception  
        {
            /*
            AUX_MU_IIR
            on read bits[2:1] :
                00 : No interrupts
                01 : Transmit holding register empty
                10 : Receiver holds valid byte
                11: <Not possible> 
            on write bits[2:1] :
                01 : Clear receive FIFO
                10 : Clear transmit FIFO
            */
            if (*AUX_MU_IIR & (1<<1))
            {
                disable_mini_uart_w_interrupt();
                add_task(uart_interrupt_w_handler, UART_IRQ_PRIORITY);
                run_preemptive_tasks();
            }
            else if (*AUX_MU_IIR & (1<<2))
            {
                disable_mini_uart_r_interrupt();
                add_task(uart_interrupt_r_handler, UART_IRQ_PRIORITY);
                run_preemptive_tasks();
            }
            else
            {
                disable_mini_uart_interrupt();
                uart_puts("uart handler error\n");
                enable_mini_uart_interrupt();
            }
        }

    }else if(*CORE0_INTERRUPT_SOURCE & (1<<1))
    {
        // disable_mini_uart_interrupt();
        // uart_putln("timer exception");
        // enable_mini_uart_interrupt();
        core_timer_disable();
        add_task(core_timer_handler, TIMER_IRQ_PRIORITY);
        run_preemptive_tasks();
        core_timer_enable();
    }
}

void invalid_exception_router(unsigned long long x0) {
    debug_exception(x0, __func__);
    INVALID_COUNT++;

    disable_mini_uart_interrupt();
    uart_puts("\nException handler not implemented\n");
    enable_mini_uart_interrupt();
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
    // disable_mini_uart_interrupt();
    // uart_puts("\t");
    // uart_hex(x0);
    // uart_puts(": ");
    // uart_puts_const(caller);
    // uart_puts(" exception\n");
    // enable_mini_uart_interrupt();
}