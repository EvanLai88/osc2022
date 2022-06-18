#include "sched.h"
#include "exception.h"
#include "malloc.h"
#include "timer.h"
#include "uart.h"
#include "signal.h"

void init_thread_sched()
{
    lock();
    run_queue = kmalloc(sizeof(list_head_t));
    wait_queue = kmalloc(sizeof(list_head_t));
    INIT_LIST_HEAD(run_queue);
    INIT_LIST_HEAD(wait_queue);

    for (int i = 0; i <= PID_MAX; i++)
    {
        threads[i].pid = i;
        threads[i].state = UNUSED;
    }

    asm volatile("msr tpidr_el1, %0" ::"r"(kmalloc(sizeof(thread_t))));

    curr_thread = thread_create(idle);
    unlock();
}

void idle(){
    while(1)
    {
        // uart_puts("idle thread.\n");
        kill_zombies();
        schedule();
    }
}

void schedule(){
    lock();
    // uart_puts("\tsched :");
    // uart_puts("\t pid ");
    // uart_int(curr_thread->pid);
    if (curr_thread->state == RUNNING)
    {
        curr_thread->state = RUNNABLE;
    }
    do{
        curr_thread = (thread_t *)curr_thread->listhead.next;
    } while (list_is_head(&curr_thread->listhead, run_queue) || curr_thread->state == ZOMBIE);
    // uart_puts(" -> ");
    // uart_puts("pid ");
    // uart_int(curr_thread->pid);
    // uart_putln("");
    curr_thread->state = RUNNING;
    switch_to(get_current(), &curr_thread->context);
    unlock();
}

void kill_zombies(){
    lock();
    list_head_t *curr;
    list_for_each(curr,run_queue)
    {
        if (((thread_t *)curr)->state == ZOMBIE)
        {
            list_del_entry(curr);
            ((thread_t *)curr)->state = UNUSED;

            kfree(((thread_t *)curr)->ustack);
            kfree(((thread_t *)curr)->kstack);
            if ( ((thread_t *)curr)->is_parent == True ) {
                kfree(((thread_t *)curr)->data);
            }
        }
    }
    unlock();
}

int exec_thread(char *data, unsigned int filesize)
{
    thread_t *thrd = thread_create(data);
    thrd->data = kmalloc(filesize);
    thrd->datasize = filesize;
    thrd->context.lr = (unsigned long)thrd->data;

    for (int i = 0; i < filesize;i++)
    {
        thrd->data[i] = data[i];
    }

    curr_thread = thrd;
    curr_thread->state = RUNNING;
    add_timer(schedule_timer, 1, "", False);

    asm("msr tpidr_el1, %0\n\t"
        "msr elr_el1, %1\n\t"
        "msr spsr_el1, xzr\n\t"
        "msr sp_el0, %2\n\t"
        "mov sp, %3\n\t"
        "eret\n\t" ::"r"(&thrd->context),"r"(thrd->context.lr), "r"(thrd->context.sp), "r"(thrd->kstack + KT_STACK_SIZE));

    return 0;
}


thread_t *thread_create(void *start)
{
    lock();

    thread_t *thrd = (thread_t *) -1;
    for (int i = 0; i <= PID_MAX; i++)
    {
        if (threads[i].state == UNUSED)
        {
            thrd = &threads[i];
            break;
        }
    }
    thrd->state = RUNNABLE;

    thrd->ustack = kmalloc(UT_STACK_SIZE);
    thrd->kstack = kmalloc(KT_STACK_SIZE);
    
    thrd->context.lr = (unsigned long long)start;
    thrd->context.sp = (unsigned long long )thrd->ustack + UT_STACK_SIZE;
    thrd->context.fp = thrd->context.sp;
    
    thrd->signal_is_checking = 0;
    for (int i = 0; i < SIGNAL_MAX;i++)
    {
        thrd->signal_handler[i] = signal_default_handler;
        thrd->sigcount[i] = 0;
    }

    list_add(&thrd->listhead, run_queue);
    unlock();
    return thrd;
}

void thread_exit(){
    lock();
    curr_thread->state = ZOMBIE;
    unlock();
    schedule();
}

void schedule_timer(char* notuse){
    unsigned long long cntfrq_el0;
    __asm__ __volatile__("mrs %0, cntfrq_el0\n\t": "=r"(cntfrq_el0));
    add_timer(schedule_timer, cntfrq_el0 >> 5, "", True);
}