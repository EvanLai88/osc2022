#include "task.h"
#include "exception.h"
#include "malloc.h"
#include "uart.h"

extern int DEBUG;

int curr_task_priority = 9999;   // init a very low priority

list_head_t *task_list;

void task_list_init()
{
    task_list = malloc(sizeof(list_head_t));
    INIT_LIST_HEAD(task_list);
}

void add_task(void *task_function,unsigned long long priority){
    int tmp = DEBUG;
    DEBUG = 0;
    task_t *the_task = kmalloc(sizeof(task_t));
    DEBUG = tmp;
    // disable_interrupt();
    // uart_hex(the_task);
    // uart_puts("\n");
    // enable_interrupt();
    the_task->priority = priority;
    the_task->task_function = task_function;

    struct list_head *curr;

    /* critical section */
    lock();
    list_for_each(curr, task_list)
    {
        if (((task_t *)curr)->priority > the_task->priority)
        {
            list_add(&the_task->listhead, curr->prev);
            break;
        }
    }
    if (list_is_head(curr, task_list))
    {
        list_add_tail(&the_task->listhead, task_list);
    }
    unlock();
}

void run_preemptive_tasks(){

    while (1)
    {
        lock();
        if (list_empty(task_list))
        {
            unlock();
            break;
        }

        task_t *the_task = (task_t *)task_list->next;
        if (curr_task_priority <= the_task->priority)
        {
            unlock();
            break;
        }

        list_del_entry((struct list_head *)the_task);
        int prev_task_priority = curr_task_priority;
        curr_task_priority = the_task->priority;
        
        unlock();
        run_task(the_task);

        curr_task_priority = prev_task_priority;
        
        int tmp = DEBUG;
        DEBUG = 0;
        kfree(the_task);
        DEBUG = tmp;
    }
}

void run_task(task_t* the_task)
{
    ((void (*)())the_task->task_function)();
}