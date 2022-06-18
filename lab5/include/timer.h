#ifndef _TIMER_H
#define _TIMER_H

#include "compiler.h"

#define CORE0_TIMER_CTRL    0x40000040
#define CORE1_TIMER_CTRL    0x40000044
#define CORE2_TIMER_CTRL    0x40000048
#define CORE3_TIMER_CTRL    0x4000004C

/* Physical Non Secure Timer */
#define CNTPNSIRQ_ENABLE    0x2
#define CNTPNSIRQ_DISABLE   0x0

#include "list.h"

typedef struct timer_event
{
    struct list_head listhead;

    unsigned long long interrupt_time; //store as tick time after cpu start

    void *callback; // interrupt -> timer_callback -> callback(args)

    char *args; // need to free the string by event callback function
} timer_event_t;

void timer_list_init();
void core_timer_enable();
void core_timer_disable();
void core_timer_interrupt_enable();
void core_timer_interrupt_disable();

void set_timeout_after(int seconds);
void set_timeout_at(unsigned long long tick);
// #define set_timeout_at(tick)        asm volatile( "msr cntp_cval_el0, %0;": "=r"(tick) )

void upTime(int newLine);
void upTime_async(int newLine);
unsigned long long get_current_tick();
unsigned long long get_timer_frq();

void timer_event_callback(timer_event_t *timer_event);
void core_timer_handler();
void two_seconds(char *arg);
void setTimeout(char *arg);
void add_timer(void *callback, unsigned long long timeout, char *args, int bytick);

#endif