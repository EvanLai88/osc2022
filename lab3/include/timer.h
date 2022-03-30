#ifndef _TIMER_H
#define _TIMER_H

#define STR(x) #x
#define XSTR(s) STR(s)

#define CORE0_TIMER_IRQ_CTRL    0x40000040

void core_timer_enable();
void core_timer_disable();

#endif