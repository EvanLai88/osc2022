#include "uart.h"
#include "shell.h"

extern char *DTB_PLACE;

void main(char *dtb)
{
    DTB_PLACE = dtb;
    uart_init();
    // uart_puts("DTB from main\n");
    // uart_hex(test);
    // uart_puts("\n");
    // uart_puts("DTB from x18\n");
    // uart_hex(dtb);
    // uart_puts("\n");
    shell();
}
