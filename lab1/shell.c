#include "shell.h"
#include "uart.h"
#include "mbox.h"
#include "lib.h"
#include "reboot.h"

void shell(){
    char cmd[BUFFER_SIZE];
    int reboot_scheduled = 0;

    shell_prompt();
    uart_int(atoi("0000123"));
    while(1){
        uart_puts("# ");
        for (int i=0; i<BUFFER_SIZE; i++) {
            cmd[i] = '\0';
        }
        uart_gets(cmd);
        uart_puts("you typed:\n");
        uart_puts(cmd);
        uart_puts("\n");

        if ( strcmp(cmd, "help") == 0 ) {
            help();
            continue;
        }

        if ( strcmp(cmd, "hello") == 0  ) {
            hello_world();
            continue;
        }
        
        if ( strcmp(cmd, "clear") == 0 ) {
            uart_puts("\033[2J\033[H");
            continue;
        }

        if ( strcmp(cmd, "reboot") == 0 ) {
            reboot_scheduled = 1;
            uart_puts("You have roughly 17 seconds to cancel reboot.\nCancel reboot with\nreboot -c\n");
            reboot(65536);
            continue;
        }

        if ( strcmp(cmd, "reboot now") == 0 ) {
            reboot(1);
            break;
        }

        if ( strcmp(cmd, "reboot -c") == 0 ) {
            if (reboot_scheduled) {
                cancel_reboot();
                reboot_scheduled = 0;
            }
            else {
                uart_puts("No scheduled reboot.\n");
            }
            continue;
        }
        
        if ( strcmp(cmd, "") == 0 ) {
            continue;
        }

        uart_puts("command not found.\n");
    }
}

void shell_prompt(){
    uart_puts("\033[2J\033[H");
    unsigned int board_revision;
    get_board_revision(&board_revision);
    uart_puts("Board revision is : 0x");
    uart_hex(board_revision);
    uart_puts("\n");
    
    unsigned int arm_mem_base_addr;
    unsigned int arm_mem_size;

    get_arm_memory_info(&arm_mem_base_addr,&arm_mem_size);
    uart_puts("ARM memory base address in bytes : 0x");
    uart_hex(arm_mem_base_addr);
    uart_puts("\n");
    uart_puts("ARM memory size in bytes : 0x");
    uart_hex(arm_mem_size);
    uart_puts("\n");

    uart_puts("\n");
    uart_puts("This is a simple shell for raspi3.\n");
    uart_puts("type help for more information\n");
}

void help(){
    uart_puts("help     : print this help menu.\n");
    uart_puts("hello    : print hello world!\n");
    uart_puts("clear    : clear screen.\n");
    uart_puts("reboot   : reboot raspberry pi.\n");

}

void hello_world(){
    uart_puts("hello world!\n");
}

void reboot(int time){
    // char buf[BUFFER_SIZE];
    // int time;
    // uart_puts("time out: ");
    // uart_gets(buf);
    // time = atoi(buf);
    // uart_puts("Set time out ");
    // uart_int(time);
    // uart_puts(" sec.\n");
    // reset(time);

    reset(time);
}

void cancel_reboot(){
    cancel_reset();
    uart_puts("reboot canceled.\n");
}