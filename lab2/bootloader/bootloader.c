#include "uart.h"
#include "lib.h"

extern unsigned long long _start;
extern unsigned long long _end;
extern unsigned long long __img_size;

void main(void *dtb)
{
    char *s;
    char *d;
    char header[BUFFER_SIZE];
    int i, length = 0;

    /* relocate bootloader */
    s = (char *) 0x80000;
    d = (char *) 0x60000;
    for (i = 0;i<&__img_size;i++) {
        *d = *s;
        s++;
        d++;
    }
    asm volatile(
        "adr    x9,     .\n"
        "sub    x9,     x9,     0x20000\n"
        "add    x9,     x9,     0x10\n"
        "br     x9\n"
    );

    uart_init();
    uart_getc(ECHO_OFF);
    uart_puts("\033[2J\033[H");
    uart_puts("Simple Bootloader\n");
    uart_puts("version: 0.7.0.alpha\n\n");
    uart_puts("DTB Base Address: ");
    uart_hex(dtb);
    uart_puts("\n");
    uart_puts("Loading kernel image from mini UART ...\n");
    uart_puts("img size :");
    
    for (i=0; i<BUFFER_SIZE; i++) {
        header[i] = '\0';
    }
    
    /* get img size */
    uart_gets(header);
    length = atoi(header);
    uart_int(length);
    uart_puts("\n");

    /* load img into memory starting at address 0x80000 */
    s = (char *) 0x80000;
    for (i = 0;i<length;i++) {
        *s = uart_getc(ECHO_OFF);
        uart_int(i);
        uart_puts(": ");
        uart_hex(*s);
        uart_puts("\n");
        s++;
    }
    // }

    /* jump to address 0x80000 */
    ((void (*)(char*))0x80000)(dtb);
}