#ifndef _TIMER_H
#define _TIMER_H

#define STR(x) #x
#define XSTR(s) STR(s)

#define CORE0_TIMER_CTRL    0x40000040
#define CORE1_TIMER_CTRL    0x40000044
#define CORE2_TIMER_CTRL    0x40000048
#define CORE3_TIMER_CTRL    0x4000004C

/* Physical Non Secure Timer */
#define CNTPNSIRQ_ENABLE    0x2
#define CNTPNSIRQ_DISABLE   0x0


void core_timer_enable();
void core_timer_disable();
void core_timer_interrupt_enable();
void core_timer_interrupt_disable();

void set_timeout_after(int seconds);
void set_timeout_at(unsigned long long tick);

void upTime();
unsigned long long get_current_tick();
unsigned long long get_timer_frq();

#endif