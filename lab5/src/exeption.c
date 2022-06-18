#include "exception.h"
#include "uart.h"
#include "timer.h"
#include "task.h"
#include "sched.h"
#include "syscall.h"
#include "signal.h"

int SYNC_64_COUNT = 0;
int IRQ_COUNT     = 0;
int INVALID_COUNT = 0;

void sync_64_router(trapframe_t* tpf)
{
    // debug_exception(x0, __func__);

    enable_interrupt();
    SYNC_64_COUNT++;

    unsigned long long syscall_no = tpf->x8;

    if (syscall_no == 0)
    {
        getpid(tpf);
    }
    else if(syscall_no == 1)
    {
        uartread(tpf,(char *) tpf->x0, tpf->x1);
    }
    else if (syscall_no == 2)
    {
        uartwrite(tpf,(char *) tpf->x0, tpf->x1);
    }
    else if (syscall_no == 3)
    {
        exec(tpf,(char *) tpf->x0, (char **)tpf->x1);
    }
    else if (syscall_no == 4)
    {
        fork(tpf);
    }
    else if (syscall_no == 5)
    {
        exit(tpf,tpf->x0);
    }
    else if (syscall_no == 6)
    {
        syscall_mbox_call(tpf,(unsigned char)tpf->x0, (unsigned int *)tpf->x1);
    }
    else if (syscall_no == 7)
    {
        kill(tpf, (int)tpf->x0);
    }
    else if (syscall_no == 8)
    {
        signal_register(tpf->x0, (void (*)())tpf->x1);
    }
    else if (syscall_no == 9)
    {
        signal_kill(tpf->x0, tpf->x1);
    }
    else if (syscall_no == 50)
    {
        sigreturn(tpf);
    }


    // unsigned long long spsr_el1;
	// asm volatile("mrs %0, SPSR_EL1\n\t" : "=r" (spsr_el1) :  : "memory");

    // unsigned long long elr_el1;
	// asm volatile("mrs %0, ELR_EL1\n\t" : "=r" (elr_el1) :  : "memory");

    // unsigned long long esr_el1;
	// asm volatile("mrs %0, ESR_EL1\n\t" : "=r" (esr_el1) :  : "memory");

    // disable_mini_uart_interrupt();
    // uart_puts("SPSR_EL1: ");
    // uart_hex(spsr_el1);
    // uart_puts("\n");
    // uart_puts("ELR_EL1: ");
    // uart_hex(elr_el1);
    // uart_puts("\n");
    // uart_puts("ESR_EL1: ");
    // uart_hex(esr_el1);
    // uart_puts("\n");
    // enable_mini_uart_interrupt();
}

void irq_router(trapframe_t* tpf)
{
    // debug_exception(x0, __func__);

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
                if (!(*AUX_MU_IER & 1)) {
                    *AUX_MU_IIR = 0xC2;
                    return;
                }
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

        if (run_queue->next->next != run_queue)schedule();
    }

    if ((tpf->spsr_el1 & 0b1100) == 0)
    {
        check_signal(tpf);
    }
}

void invalid_exception_router(unsigned long long x0) {

    disable_interrupt();
    uart_puts("\nException handler not implemented\n");

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
    while(1);
}

void enable_interrupt(){
    asm volatile("msr daifclr, 0xf");
}

void disable_interrupt(){
    asm volatile("msr daifset, 0xf");
}

static unsigned long long lock_count = 0;
void lock()
{
    disable_interrupt();
    lock_count++;
}

void unlock()
{
    lock_count--;
    if (lock_count<0)
    {
        uart_printf("lock error !!!\r\n");
        while(1);
    }
    if (lock_count == 0)
        enable_interrupt();
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