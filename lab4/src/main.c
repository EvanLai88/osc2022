#include "uart.h"
#include "shell.h"
#include "exception.h"
#include "timer.h"
#include "task.h"
#include "malloc.h"

extern char *DTB_PLACE;

void main(char *dtb)
{
    DTB_PLACE = dtb;
    uart_init();
    uart_getc(ECHO_OFF);
    
    kmalloc_init();
    task_list_init();
    timer_list_init();
    enable_interrupt();
    core_timer_enable();
    enable_mini_uart_interrupt();
    shell();
}
