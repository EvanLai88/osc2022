#ifndef SCHED_H
#define SCHED_H

#include "list.h"

#define PID_MAX             32768
#define SIGNAL_MAX          64

#define UT_STACK_SIZE       0x10000
#define KT_STACK_SIZE       0x10000

#define UNUSED              0x0000
#define RUNNING             0x0001
#define RUNNABLE            0x0002
#define SLEEP               0x0003
#define INTERRUPTABLE       0x0004
#define UNINTERRUPTABLE     0x0005
#define ZOMBIE              0xffff

#define PRIORITY_HIGH       0x0000
#define PRIORITY_MEDIUM     0x0001
#define PRIORITY_LOW        0x0002


extern void switch_to(void *curr_context, void *next_context);
extern void store_context(void *curr_context);
extern void load_context(void *curr_context);
extern void *get_current();

typedef struct thread_context
{
    unsigned long x19;
    unsigned long x20;
    unsigned long x21;
    unsigned long x22;
    unsigned long x23;
    unsigned long x24;
    unsigned long x25;
    unsigned long x26;
    unsigned long x27;
    unsigned long x28;
    unsigned long fp;
    unsigned long lr;
    unsigned long sp;
} thread_context_t;

typedef struct thread
{
    list_head_t         listhead;
    int                 pid;
    // int                 state;
    // int                 is_parent;
    int                 signal_is_checking;
    int                 isused;
    int                 iszombie;

    void                (*signal_handler[SIGNAL_MAX+1])();
    int                 sigcount[SIGNAL_MAX + 1];
    void                (*curr_signal_handler)();
    thread_context_t    signal_saved_context;
    
    char                *data;          // need to be freed
    unsigned long       datasize;
    char                *ustack;        // need to be freed
    char                *kstack;        // need to be freed
    thread_context_t    context;
    
} thread_t;

thread_t *curr_thread;
list_head_t *run_queue;
list_head_t *wait_queue;

thread_t threads[PID_MAX + 1];

void schedule_timer(char *notuse);
void init_thread_sched();
void idle();
void schedule();
void kill_zombies();
void thread_exit();
thread_t *thread_create(void *start);
int exec_thread(char *data, unsigned int filesize);

#endif