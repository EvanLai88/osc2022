#include "timer.h"
#include "uart.h"

void core_timer_enable()
{
    asm volatile (
        // enable
        "mov x1, 1;"
        "msr cntp_ctl_el0, x1;"
    );
    // core_timer_interrupt_enable();
}

void core_timer_disable()
{
    asm volatile (
        // disable
        "mov x1, 0;"
        "msr cntp_ctl_el0, x1;"
    );
    // core_timer_interrupt_disable();
}

void core_timer_interrupt_enable()
{
    asm volatile (
        // unmask timer interrupt
        "mov x1, " XSTR(CNTPNSIRQ_ENABLE) ";"
        "ldr x2, =" XSTR(CORE0_TIMER_CTRL) ";"
        "str w1, [x2];"
    );
}

void core_timer_interrupt_disable()
{
    asm volatile (
        // mask timer interrupt
        "mov x1, " XSTR(CNTPNSIRQ_DISABLE) ";"
        "ldr x2, =" XSTR(CORE0_TIMER_CTRL) ";"
        "str w1, [x2];"
    );
}

void set_timeout_after(int seconds)
{
    asm volatile(
        "mrs x1, cntfrq_el0;"
        "mul x1, x1, %0;"
        "msr cntp_tval_el0, x1;"
        : "=r"(seconds)
    );
}

void set_timeout_at(unsigned long long tick)
{
    asm volatile(
        "msr cntp_cval_el0, %0;"
        : "=r"(tick)
    );
}

void upTime()
{
    unsigned long long current_tick, cnt_frq;
    current_tick = get_current_tick();
    cnt_frq = get_timer_frq();
    
    uart_puts("\nUp time: ");
    uart_int(current_tick/cnt_frq);
    uart_puts(" seconds\n");
}

unsigned long long get_current_tick()
{
    unsigned long long current_tick;
    asm volatile("mrs %0, cntpct_el0\n\t": "=r"(current_tick));
    return current_tick;
}

unsigned long long get_timer_frq()
{
    unsigned long long cnt_frq;
    asm volatile("mrs %0, cntfrq_el0\n\t": "=r"(cnt_frq));
    return cnt_frq;
}



// core_timer_handler:
//   mrs x0, cntfrq_el0
//   msr cntp_tval_el0, x0