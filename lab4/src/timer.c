#include "timer.h"
#include "uart.h"
#include "malloc.h"
#include "lib.h"

list_head_t *timer_event_list;
int two_second_recurrent = 0;

void timer_list_init()
{
    timer_event_list = malloc(sizeof(list_head_t));
    INIT_LIST_HEAD(timer_event_list);
}

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
    asm volatile (
        "msr cntp_cval_el0, %0;"
        :: "r"(tick)
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

unsigned long long get_tick_after(int seconds)
{
    return get_current_tick() + (get_timer_frq() * seconds);
}

void timer_event_callback(timer_event_t *timer_event)
{
    upTime();
    ((void (*)(char *))timer_event->callback)(timer_event->args);
    list_del_entry((struct list_head *)timer_event);
    free(timer_event->args);
    free(timer_event);

    if (!list_empty(timer_event_list)) {
        set_timeout_at(((timer_event_t *)timer_event_list->next)->interrupt_time);
    }
    else {
        set_timeout_after(10000);
    }
}

void core_timer_handler()
{
    if (list_empty(timer_event_list))
    {
        set_timeout_after(10000);
        return;
    }

    timer_event_callback((timer_event_t *)timer_event_list->next);
}

void two_seconds(char *arg)
{
    if (two_second_recurrent == 1) {
        uart_putln(arg);
        add_timer(two_seconds, 2, arg);
    }
    else {
        set_timeout_after(10000);
    }
}

void add_timer(void *callback, unsigned long long timeout, char *args)
{

    timer_event_t *the_timer_event = malloc(sizeof(timer_event_t));

    // store argument string into timer_event
    the_timer_event->args = malloc(len(args) + 1);
    strcpy(the_timer_event->args, args);

    the_timer_event->interrupt_time = get_tick_after(timeout);
    the_timer_event->callback = callback;
    INIT_LIST_HEAD(&the_timer_event->listhead);

    // add the timer_event into timer_event_list (sorted)
    list_head_t *curr;
    list_for_each(curr, timer_event_list)
    {
        if (((timer_event_t *)curr)->interrupt_time > the_timer_event->interrupt_time)
        {
            list_add(&the_timer_event->listhead, curr->prev);
            break;
        }
    }

    if (list_is_head(curr, timer_event_list))
    {
        list_add_tail(&the_timer_event->listhead, timer_event_list); // for the time is the biggest
    }
    set_timeout_at(((timer_event_t *)timer_event_list->next)->interrupt_time);
}