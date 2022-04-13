#include "task.h"
#include "exception.h"
#include "malloc.h"
#include "uart.h"

int curr_task_priority = 9999;   // init a very low priority

list_head_t *task_list;

void task_list_init()
{
    task_list = malloc(sizeof(list_head_t));
    INIT_LIST_HEAD(task_list);
}

void add_task(void *task_function,unsigned long long priority){
    task_t *the_task = malloc(sizeof(task_t));

    the_task->priority = priority;
    the_task->task_function = task_function;
    INIT_LIST_HEAD(&the_task->listhead);

    struct list_head *curr;

    /* critical section */
    disable_interrupt();
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
    enable_interrupt();
}

void run_preemptive_tasks(){
    enable_interrupt();
    while (!list_empty(task_list))
    {
        /* critical section */
        disable_interrupt();
        task_t *the_task = (task_t *)task_list->next;
        if (curr_task_priority <= the_task->priority)
        {
            enable_interrupt();
            break;
        }

        list_del_entry((struct list_head *)the_task);
        int prev_task_priority = curr_task_priority;
        curr_task_priority = the_task->priority;
        enable_interrupt();

        run_task(the_task);

        /* critical section */
        disable_interrupt();
        curr_task_priority = prev_task_priority;
        enable_interrupt();
        free(the_task);
    }
}

void run_task(task_t* the_task)
{
    ((void (*)())the_task->task_function)();
}