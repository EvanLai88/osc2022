#include "task.h"
#include "exception.h"
#include "malloc.h"
#include "uart.h"

/*
Preemption
Now, any interrupt handler can preempt the task’s execution, but the newly enqueued task still needs to wait for the currently running task’s completion. It’d be better if the newly enqueued task with a higher priority can preempt the currently running task.

To achieve the preemption, the kernel can check the last executing task’s priority before returning to the previous interrupt handler. If there are higher priority tasks, execute the highest priority task.
*/
int curr_task_priority = 9999;   // init a very low priority

struct list_head *task_list;

void task_list_init()
{
    INIT_LIST_HEAD(task_list);
}

//like add_timer
void add_task(void *task_function,unsigned long long priority){
    task_t *the_task = malloc(sizeof(task_t)); //need to free by task runner

    the_task->priority = priority; // store interrupt time into timer_event
    the_task->task_function = task_function;
    INIT_LIST_HEAD(&the_task->listhead);

    // add the timer_event into timer_event_list (sorted) 
    // if the same priority FIFO
    struct list_head *curr;

    disable_interrupt(); // critical section
    list_for_each(curr, task_list)
    {
        if (((task_t *)curr)->priority > the_task->priority)
        {
            list_add(&the_task->listhead, curr->prev); // add this timer at the place just before the bigger one (sorted)
            break;
        }
    }

    if (list_is_head(curr, task_list))
    {
        list_add_tail(&the_task->listhead, task_list); // for the time is the biggest
    }
    enable_interrupt();
}

void run_preemptive_tasks(){
    enable_interrupt(); //do the tasks with interrupts enabled, (lab3 advanced 2 )
    while (!list_empty(task_list))
    {
        disable_interrupt();  // critical section
        task_t *the_task = (task_t *)task_list->next;
        // just run task when priority is lower than the task preempted
        if (curr_task_priority <= the_task->priority)
        {
            enable_interrupt();
            break; //
        }
        list_del_entry((struct list_head *)the_task);
        int prev_task_priority = curr_task_priority;
        curr_task_priority = the_task->priority;
        enable_interrupt();

        run_task(the_task);

        disable_interrupt(); // critical section
        curr_task_priority = prev_task_priority;
        enable_interrupt();
        free(the_task);
    }
}

void run_task(task_t* the_task)
{
    ((void (*)())the_task->task_function)();
}