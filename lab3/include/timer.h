#ifndef _TIMER_H
#define _TIMER_H

#include "list.h"

#define STR(x) #x
#define XSTR(s) STR(s)

#define CORE0_TIMER_IRQ_CTRL    0x40000040

typedef struct timer_event
{
    struct list_head listhead;

    unsigned long long interrupt_time; //store as tick time after cpu start

    void *callback; // interrupt -> timer_callback -> callback(args)

    char *args; // need to free the string by event callback function
} timer_event_t;

void timer_list_init();
void timer_event_callback(timer_event_t *timer_event);
void core_timer_handler();
void add_timer(void *callback, unsigned long long timeout, char *args);
void core_timer_enable();
void core_timer_disable();
void two_second_timeout();
void two_second_alert(char *str);
unsigned long long get_tick_plus_s(unsigned long long second);
void set_core_timer_interrupt(unsigned long long expired_time);
void set_core_timer_interrupt_by_tick(unsigned long long tick);

#endif