#include "exe.h"
#include "malloc.h"
#include "uart.h"

int exe(void *file) {
    
    void *user_stack = malloc(USER_STACK_SIZE);
    // uart_hex(USER_STACK_SIZE);
    // uart_puts("\n");
    // uart_hex((unsigned long long)file);
    // uart_puts("\n");
    // uart_hex((unsigned long long)user_stack);
    // uart_puts("\n");

    asm(
        /* EL1h (SPSel = 1) with interrupt disabled */
        "mov x0, 0x3c0;"
        "msr spsr_el1, x0;"

        /* set user programe address */
        "msr elr_el1, %0;"

        /* set user stack address */
        "msr sp_el0,  %1;"

        /* return to EL1 */
        "eret;"
        ::  "r" (file),
            "r" (user_stack+USER_STACK_SIZE)     // stack grow from high to low
    );

    return 0;
}