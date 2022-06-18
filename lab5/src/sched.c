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

    //init pids
    for (int i = 0; i <= PID_MAX; i++)
    {
        threads[i].isused = 0;
        threads[i].pid = i;
        threads[i].iszombie = 0;
    }

    asm volatile("msr tpidr_el1, %0" ::"r"(kmalloc(sizeof(thread_t)))); /// malloc a space for current kernel thread to prevent crash

    thread_t* idlethread = thread_create(idle);
    curr_thread = idlethread;
    unlock();
}

void idle(){
    while(1)
    {
        // uart_printf("idle thread.\n");
        kill_zombies();   //reclaim threads marked as DEAD
        schedule();       //switch to next thread in run queue
    }
}

void schedule(){
    lock();
    // uart_puts("\tsched :");
    // uart_puts("\t pid ");
    // uart_int(curr_thread->pid);
    do{
        curr_thread = (thread_t *)curr_thread->listhead.next;
    } while (list_is_head(&curr_thread->listhead, run_queue) || curr_thread->iszombie);
    // uart_puts(" -> ");
    // uart_puts("pid ");
    // uart_int(curr_thread->pid);
    // uart_putln("");
    switch_to(get_current(), &curr_thread->context);
    unlock();
}

void kill_zombies(){
    lock();
    list_head_t *curr;
    list_for_each(curr,run_queue)
    {
        if (((thread_t *)curr)->iszombie)
        {
            list_del_entry(curr);
            kfree(((thread_t *)curr)->ustack);        // free stack
            kfree(((thread_t *)curr)->kstack); // free stack
            //kfree(((thread_t *)curr)->data); // free data (don't free data because of fork)
            ((thread_t *)curr)->iszombie = 0;
            ((thread_t *)curr)->isused = 0;
        }
    }
    unlock();
}

int exec_thread(char *data, unsigned int filesize)
{
    thread_t *t = thread_create(data);
    t->data = kmalloc(filesize);
    t->datasize = filesize;
    t->context.lr = (unsigned long)t->data;
    //copy file into data
    for (int i = 0; i < filesize;i++)
    {
        t->data[i] = data[i];
    }

    //disable echo when going to userspace
    // uart_disable_echo();
    curr_thread = t;
    add_timer(schedule_timer, 1, "", 0);
    // eret to exception level 0
    asm("msr tpidr_el1, %0\n\t"
        "msr elr_el1, %1\n\t"
        "msr spsr_el1, xzr\n\t" // enable interrupt in EL0. You can do it by setting spsr_el1 to 0 before returning to EL0.
        "msr sp_el0, %2\n\t"
        "mov sp, %3\n\t"
        "eret\n\t" ::"r"(&t->context),"r"(t->context.lr), "r"(t->context.sp), "r"(t->kstack + KT_STACK_SIZE));

    return 0;
}


//malloc a kstack and a userstack
thread_t *thread_create(void *start)
{
    lock();

    thread_t *r = (thread_t *) 0;
    for (int i = 0; i <= PID_MAX; i++)
    {
        if (!threads[i].isused)
        {
            r = &threads[i];
            break;
        }
    }
    r->iszombie = 0;
    r->isused = 1;
    r->context.lr = (unsigned long long)start;
    r->ustack = kmalloc(UT_STACK_SIZE);
    r->kstack = kmalloc(KT_STACK_SIZE);
    r->context.sp = (unsigned long long )r->ustack + UT_STACK_SIZE;
    r->context.fp = r->context.sp;
    r->signal_is_checking = 0;
    //initial signal handler with signal_default_handler (kill thread)
    for (int i = 0; i < SIGNAL_MAX;i++)
    {
        r->signal_handler[i] = signal_default_handler;
        r->sigcount[i] = 0;
    }

    list_add(&r->listhead, run_queue);
    unlock();
    return r;
}

void thread_exit(){
    lock();
    curr_thread->iszombie = 1;
    unlock();
    schedule();
}

void schedule_timer(char* notuse){
    unsigned long long cntfrq_el0;
    __asm__ __volatile__("mrs %0, cntfrq_el0\n\t": "=r"(cntfrq_el0)); //tick frequency
    add_timer(schedule_timer, cntfrq_el0 >> 5, "", 1);
}