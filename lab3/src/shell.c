#include "shell.h"
#include "uart.h"
#include "mbox.h"
#include "lib.h"
#include "reboot.h"
#include "cpio.h"
#include "fdt.h"
#include "malloc.h"
#include "cpio.h"
#include "exe.h"
#include "timer.h"

extern char* DTB_PLACE;

void shell(){
    char cmd[BUFFER_SIZE];
    int reboot_scheduled = 0;

    shell_prompt();
    while(1){
        uart_async_puts("# ");
        for (int i=0; i<BUFFER_SIZE; i++) {
            cmd[i] = '\0';
        }
        uart_async_gets(cmd);
        // uart_async_puts("you typed:\n");
        // uart_async_puts(cmd);
        // uart_async_puts("\n");

        if ( strcmp(cmd, "help") == 0 ) {
            help();
            continue;
        }

        if ( strcmp(cmd, "hello") == 0  ) {
            hello_world();
            continue;
        }
        
        if ( strcmp(cmd, "ls") == 0  ) {
            ls();
            continue;
        }
        
        if ( strcmp(cmd, "cat") == 0  ) {
            uart_async_puts("filename: ");
            uart_async_gets(cmd);
            cat(cmd);
            continue;
        }

        if ( strcmp(cmd, "clear") == 0 ) {
            uart_async_puts("\033[2J\033[H");
            continue;
        }

        if ( strcmp(cmd, "exe") == 0 ) {
            uart_async_puts("filename: ");
            uart_async_gets(cmd);
            exefile(cmd);
            continue;
        }

        if (strncmp(cmd, "setTimeout", sizeof("setTimeout") - 1) == 0)
        {
            char *message = strchr(cmd, ' ') + 1;
            char *end_message = strchr(message, ' ');
            *end_message = '\0';
            char *seconds = end_message + 1;
            add_timer(uart_putln, atoi(seconds), message);
            continue;
        }

        if (strcmp(cmd, "clockAlert") == 0)
        {
            add_timer(two_second_alert, 2, "two_second_alert");
            continue;
        }

        if ( strcmp(cmd, "reboot") == 0 ) {
            reboot_scheduled = 1;
            // uart_async_puts("You have roughly 17 seconds to cancel reboot.\nCancel reboot with\nreboot -c\n");

            int time;
            uart_async_puts("time out: ");
            uart_async_gets(cmd);
            time = atoi(cmd);
            uart_async_puts("Set time out ");
            uart_int(time);
            uart_async_puts(" sec.\n");
            reboot(time);
            continue;
        }

        if ( strcmp(cmd, "reboot now") == 0 ) {
            reboot(1);
            break;
        }

        if ( strcmp(cmd, "dtb list -a") == 0 ) {
            uart_async_puts("dtb\n");
            traverse_device_tree(DTB_PLACE,dtb_callback_show_tree);
            continue;
        }

        if ( strcmp(cmd, "reboot -c") == 0 ) {
            if (reboot_scheduled) {
                cancel_reboot();
                reboot_scheduled = 0;
            }
            else {
                uart_async_puts("No scheduled reboot.\n");
            }
            continue;
        }
        
        if ( strcmp(cmd, "") == 0 ) {
            continue;
        }

        uart_async_puts("command not found.\n");
    }
}

void shell_prompt(){
    traverse_device_tree(DTB_PLACE,dtb_callback_initramfs);
    uart_async_puts("\n");
    uart_async_puts("\033[2J\033[H");
    unsigned int board_revision;
    get_board_revision(&board_revision);
    uart_async_puts("Board revision is : 0x");
    uart_hex(board_revision);
    uart_async_puts("\n");
    
    unsigned int arm_mem_base_addr;
    unsigned int arm_mem_size;

    get_arm_memory_info(&arm_mem_base_addr,&arm_mem_size);
    uart_async_puts("ARM memory base address in bytes : 0x");
    uart_hex(arm_mem_base_addr);
    uart_async_puts("\n");
    uart_async_puts("ARM memory size in bytes : 0x");
    uart_hex(arm_mem_size);
    uart_async_puts("\n");
    uart_async_puts("DTB base address: ");
    uart_hex(DTB_PLACE);
    uart_async_puts("\n");
    
    char* string = malloc(8);

    uart_async_puts("malloc(8) address:");
    uart_hex(string);
    uart_async_puts("\n");

    string = malloc(4);
    uart_async_puts("malloc(8) address:");
    uart_hex(string);
    uart_async_puts("\n");

    uart_async_puts("\n");
    uart_async_puts("This is a simple shell for raspi3.\n");
    uart_async_puts("type help for more information\n");
}

void help(){
    uart_async_puts("help     : print this help menu.\n");
    uart_async_puts("ls       : list files.\n");
    uart_async_puts("cat      : print file content.\n");
    uart_async_puts("hello    : print hello world!\n");
    uart_async_puts("clear    : clear screen.\n");
    uart_async_puts("reboot   : reboot raspberry pi.\n");
}

void hello_world(){
    uart_async_puts("hello world!\n");
}

void reboot(int time){
    // char buf[BUFFER_SIZE];
    // int time;
    // uart_async_puts("time out: ");
    // uart_async_gets(buf);
    // time = atoi(buf);
    // uart_async_puts("Set time out ");
    // uart_int(time);
    // uart_async_puts(" sec.\n");
    // reset(time);

    reset(time << 16);
}

void cancel_reboot(){
    cancel_reset();
    uart_async_puts("reboot canceled.\n");
}

void ls(){
    cpio_ls(CPIO_BASE);
}

void cat(char *filename){
    cpio_cat(CPIO_BASE, filename );
}

void exefile(void *filename) {
    const char *current_filename;
    struct cpio_header *header, *next;
    void *result;
    int error;
    unsigned long size;
    int exist = 0;

    header = CPIO_BASE;
    while(header != 0) {
        error = cpio_parse_header(header, &current_filename, &size, &result, &next);
        if (error == 1) {
            break;
        }
        if (cpio_strncmp( filename, current_filename, cpio_strlen(filename)) == 0) {
            exist = 1;
            uart_hex(result);
            uart_async_puts("\n");
            exe(result);
            return;
        }
        header = next;
    }
    if (exist == 0) {
        uart_async_puts("File does not exists.\n");
    }
}