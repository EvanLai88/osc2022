#include "timer.h"


void core_timer_enable()
{
    asm volatile(
        "mov x1, 1;"
        "msr cntp_ctl_el0, x1;" // enable
        "mrs x1, cntfrq_el0;"
        "msr cntp_tval_el0, x1;" // set expired time

        "mov x2, 2;"
        "ldr x1, =" XSTR(CORE0_TIMER_IRQ_CTRL) ";"
        "str w2, [x1];" // unmask timer interrupt
    );
}

void core_timer_disable()
{
    asm volatile(
        "mov x2, 0;"
        "ldr x1, =" XSTR(CORE0_TIMER_IRQ_CTRL) ";"
        "str w2, [x1];" // unmask timer interrupt
    );
}