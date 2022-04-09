#include "uart.h"
#include "shell.h"
#include "exception.h"
#include "timer.h"

extern char *DTB_PLACE;

void main(char *dtb)
{
    DTB_PLACE = dtb;
    uart_init();
    uart_getc(ECHO_OFF);
    enable_interrupt();
    core_timer_enable();
    shell();
}
