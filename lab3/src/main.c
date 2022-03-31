#include "uart.h"
#include "shell.h"
#include "exceptionHandler.h"
#include "timer.h"

extern char *DTB_PLACE;

void main(char *dtb)
{
    DTB_PLACE = dtb;
    uart_init();
    timer_list_init();
    core_timer_init();
    
    enable_mini_uart_interrupt();
    enable_interrupt();
    shell();
}
