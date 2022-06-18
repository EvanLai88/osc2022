#include "syscall.h"
#include "sched.h"
#include "cpio.h"
#include "uart.h"
#include "exception.h"
#include "malloc.h"
#include "mbox.h"
#include "signal.h"
#include "cpio.h"

int getpid(trapframe_t* tpf)
{
    tpf->x0 = curr_thread->pid;
    return curr_thread->pid;
}

size_t uartread(trapframe_t *tpf,char buf[], size_t size)
{
    int i = 0;
    for (int i = 0; i < size;i++)
    {
        uart_disable_echo();
        buf[i] = uart_async_getc();
        uart_enable_echo();
    }
    tpf->x0 = i;
    return i;
}

size_t uartwrite(trapframe_t *tpf,const char buf[], size_t size)
{
    int i = 0;
    for (int i = 0; i < size; i++)
    {
        uart_async_putc(buf[i]);
    }
    tpf->x0 = i;
    return i;
}

//In this lab, you won’t have to deal with argument passing
int exec(trapframe_t *tpf,const char *name, char *const argv[])
{
    curr_thread->datasize = cpio_file_size((char*)name);
    char *new_data = cpio_file_start((char *)name);
    for (unsigned int i = 0; i < curr_thread->datasize;i++)
    {
        curr_thread->data[i] = new_data[i];
    }

    //clear signal handler
    for (int i = 0; i <= SIGNAL_MAX; i++)
    {
        curr_thread->signal_handler[i] = signal_default_handler;
    }

    tpf->elr_el1 = (unsigned long)curr_thread->data;
    tpf->sp_el0 = (unsigned long)curr_thread->ustack + UT_STACK_SIZE;
    tpf->x0 = 0;
    return 0;
}

int fork(trapframe_t *tpf)
{
    lock();
    thread_t *newt = thread_create(curr_thread->data);

    //copy signal handler
    for (int i = 0; i <= SIGNAL_MAX;i++)
    {
        newt->signal_handler[i] = curr_thread->signal_handler[i];
    }

    newt->datasize = curr_thread->datasize;
    int parent_pid = curr_thread->pid;
    thread_t *parent_thread_t = curr_thread;

    //Cannot copy data because there are a lot of ret addresses on stack
    //copy user stack into new process
    for (int i = 0; i < UT_STACK_SIZE; i++)
    {
        newt->ustack[i] = curr_thread->ustack[i];
    }

    //copy stack into new process
    for (int i = 0; i < KT_STACK_SIZE; i++)
    {
        newt->kstack[i] = curr_thread->kstack[i];
    }
    
    // uart_putln("111");
    store_context(get_current());
    // uart_putln("222");
    // for child
    if( parent_pid != curr_thread->pid)
    {
        goto child;
    }
    // uart_putln("333");

    newt->context = curr_thread->context;
    unsigned long offset = newt->kstack - curr_thread->kstack;
    newt->context.fp += offset;
    newt->context.sp += offset;

    unlock();

    tpf->x0 = newt->pid;
    return newt->pid;

child:
    tpf = (trapframe_t*)((char *)tpf + (unsigned long)newt->kstack - (unsigned long)parent_thread_t->kstack); // move tpf
    tpf->sp_el0 += newt->ustack - parent_thread_t->ustack;
    tpf->x0 = 0;
    return 0;
}

void exit(trapframe_t *tpf, int status)
{
    thread_exit();
}

int syscall_mbox_call(trapframe_t *tpf, unsigned char ch, unsigned int *mbox)
{
    lock();
    unsigned long r = (((unsigned long)((unsigned long)mbox) & ~0xF) | (ch & 0xF));
    /* wait until we can write to the mailbox */
    do{asm volatile("nop");} while (*MBOX_STATUS & MBOX_FULL);
    /* write the address of our message to the mailbox with channel identifier */
    *MBOX_WRITE = r;
    /* now wait for the response */
    while (1)
    {
        /* is there a response? */
        do
        {
            asm volatile("nop");
        } while (*MBOX_STATUS & MBOX_EMPTY);
        /* is it a response to our message? */
        if (r == *MBOX_READ)
        {
            /* is it a valid successful response? */
            tpf->x0 = (mbox[1] == MBOX_RESPONSE);
            unlock();
            return mbox[1] == MBOX_RESPONSE;
        }
    }

    tpf->x0 = 0;
    unlock();
    return 0;
}

void kill(trapframe_t *tpf,int pid)
{
    lock();
    if (pid >= PID_MAX || pid < 0  || !threads[pid].isused)
    {
        unlock();
        return;
    }
    threads[pid].iszombie = 1;
    unlock();
    schedule();
}

void signal_register(int signal, void (*handler)())
{
    if (signal > SIGNAL_MAX || signal < 0)return;

    curr_thread->signal_handler[signal] = handler;
}

void signal_kill(int pid, int signal)
{
    if (pid > PID_MAX || pid < 0 || !threads[pid].isused)return;

    lock();
    threads[pid].sigcount[signal]++;
    unlock();
}

void sigreturn(trapframe_t *tpf)
{
    unsigned long signal_ustack = 0;
    if (tpf->sp_el0 % UT_STACK_SIZE == 0) {
        signal_ustack = tpf->sp_el0 - UT_STACK_SIZE;
    }
    else {
        signal_ustack = tpf->sp_el0 & (~(UT_STACK_SIZE -1));
    }

    kfree((char*)signal_ustack);
    load_context(&curr_thread->signal_saved_context);
}
