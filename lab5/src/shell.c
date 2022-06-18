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
#include "exception.h"
#include "sched.h"

extern char* DTB_PLACE;
extern int two_second_recurrent;
extern uint32_t arm_memory_base;
extern uint32_t arm_memory_size;
extern int DEBUG;

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
        // uart_puts("you typed:\n");
        // uart_puts(cmd);
        // uart_puts("\n");

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
            upTime(NO_NEW_LINE);
            add_timer(setTimeout, atoi(seconds), message, False);
            continue;
        }

        if ( strcmp(cmd, "two_second") == 0 ) {
            two_second_recurrent = 1;
            add_timer(two_seconds, 2, "two_second!!", False);
            continue;
        }

        if ( strcmp(cmd, "two_second -c") == 0 ) {
            two_second_recurrent = 0;
            continue;
        }

        if ( strcmp(cmd, "clear") == 0 ) {
            uart_async_puts("\033[2J\033[H");
            continue;
        }

        if ( strcmp(cmd, "reboot") == 0 ) {
            reboot_scheduled = 1;
            // uart_puts("You have roughly 17 seconds to cancel reboot.\nCancel reboot with\nreboot -c\n");

            int time;
            uart_async_puts("time out: ");
            uart_async_gets(cmd);
            time = atoi(cmd);
            uart_async_puts("Set time out ");
            uart_async_int(time);
            uart_async_puts(" sec.\n");
            reboot(time);
            continue;
        }

        if ( strcmp(cmd, "reboot now") == 0 ) {
            reboot(1);
            break;
        }

        if ( strcmp(cmd, "test page") == 0 ) {
            test_page();
            continue;
        }

        if ( strcmp(cmd, "test chunk") == 0 ) {
            test_chunk();
            continue;
        }

        if ( strcmp(cmd, "test kmalloc") == 0 ) {
            test_kmalloc();
            continue;
        }

        if ( strcmp(cmd, "test reserve") == 0 ) {
            test_reserve();
            continue;
        }

        if ( strcmp(cmd, "test timer") == 0 ) {
            uart_async_puts("\r");
            upTime(NO_NEW_LINE);
            uart_async_puts("setTimeout aaa 30\n");
            add_timer(setTimeout, 30, "aaa", False);
            uart_async_puts("setTimeout bbb 20\n");
            add_timer(setTimeout, 20, "bbb", False);
            uart_async_puts("setTimeout ccc 25\n");
            add_timer(setTimeout, 25, "ccc", False);
            uart_async_puts("setTimeout ddd 15\n");
            add_timer(setTimeout, 15, "ddd", False);
            uart_async_puts("setTimeout eee 10\n");
            add_timer(setTimeout, 10, "eee", False);
            uart_async_puts("setTimeout fff 5\n");
            add_timer(setTimeout, 5,  "fff", False);
            continue;
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
    uart_getc();
    uart_puts("\n");
    uart_puts("\033[2J\033[H");
    unsigned int board_revision;
    get_board_revision(&board_revision);
    uart_puts("Board revision is : 0x");
    uart_hex(board_revision);
    uart_puts("\n");

    uart_puts("ARM memory base in bytes : 0x");
    uart_hex(arm_memory_base);
    uart_puts("\n");
    uart_puts("ARM memory size in bytes : 0x");
    uart_hex(arm_memory_size);
    uart_puts("\n");
    uart_puts("DTB base address: 0x");
    uart_hex((unsigned long long)DTB_PLACE);
    uart_puts("\n");
    
    // char* string = malloc(8);

    // uart_puts("malloc(8) address: 0x");
    // uart_hex((unsigned long long)string);
    // uart_puts("\n");

    // string = malloc(4);
    // uart_puts("malloc(8) address: 0x");
    // uart_hex((unsigned long long)string);
    // uart_puts("\n");

    uart_puts("\n");
    uart_puts("This is a simple shell for raspi3.\n");
    uart_puts("type help for more information\n");
    upTime(NO_NEW_LINE);
    core_timer_interrupt_enable();
}

void help(){
    uart_async_puts("help                       : print this help menu.\n");
    uart_async_puts("ls                         : list files.\n");
    uart_async_puts("cat                        : print file content.\n");
    uart_async_puts("hello                      : print hello world!\n");
    uart_async_puts("clear                      : clear screen.\n");
    uart_async_puts("dtb list -a                : list device tree.\n\n");
    uart_async_puts("reboot                     : reboot raspberry pi.\n");
    uart_async_puts("reboot now                 : reboot raspberry pi immediately.\n");
    uart_async_puts("reboot -c                  : cancel reboot.\n\n");
    uart_async_puts("two_second                 : start two second interrupt.\n");
    uart_async_puts("two_second -c              : cancel two second interrupt.\n");
    uart_async_puts("setTimeout <sec> <msg>     : test timer multiplex.\n");
    uart_async_puts("test timer                 : test timer multiplex.\n");
    uart_async_puts("test page                  : test buddy system page allocation.\n");
    uart_async_puts("test chunk                 : test chunk size allocation.\n");
    uart_async_puts("test reserve               : test malloc init reserve memory.\n");

}

void hello_world(){
    uart_async_puts("hello world!\n");
}

void reboot(int time){
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
    char *current_filename;
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
        if (cpio_strncmp( filename, current_filename, cpio_strlen(current_filename)) == 0) {
            exist = 1;
            uart_async_puts(filename);
            uart_async_puts(" address: ");
            uart_async_hex((unsigned long long)result);
            uart_async_puts("\n");
            exec_thread(result, size);
            return;
        }
        header = next;
    }
    if (exist == 0) {
        uart_async_puts("File does not exists.\n");
    }
}

void check_all_free_list()
{
    for (int i=0; i<=MAX_ORDER; i++)
    {
        show_free_list(i);
    }
}

void test_page()
{
    DEBUG = 1;
    disable_interrupt();
    // check_all_free_list();
    uart_puts("allocate 0x0f00\n");
    void *a = alloc_pages(0x0f00);
    uart_puts("address: 0x");
    uart_hex((unsigned long long)a);
    uart_puts("\n");
    // check_all_free_list();
    uart_getc();
    uart_puts("free 0x");
    uart_hex((unsigned long long)a);
    uart_puts("\n");
    free_pages(a);
    // check_all_free_list();

    uart_getc();
    uart_puts("\nallocate 0x1000\n");
    void *b = alloc_pages(0x1000);
    uart_hex((unsigned long long)b);
    uart_puts("\n");
    // check_all_free_list();
    uart_getc();
    uart_puts("free 0x");
    uart_hex((unsigned long long)b);
    uart_puts("\n");
    free_pages(b);
    // check_all_free_list();
    
    uart_getc();
    uart_puts("\nallocate 0x1100\n");
    void *c = alloc_pages(0x1100);
    uart_hex((unsigned long long)c);
    uart_puts("\n");
    // check_all_free_list();
    uart_puts("free 0x");
    uart_hex((unsigned long long)c);
    uart_puts("\n");
    free_pages(c);
    // check_all_free_list();
    
    uart_getc();
    uart_puts("\nallocate 0x4000\n");
    void *d = alloc_pages(0x4000);
    uart_hex((unsigned long long)d);
    uart_puts("\n");
    // check_all_free_list();
    uart_puts("free 0x");
    uart_hex((unsigned long long)d);
    uart_puts("\n");
    free_pages(d);
    // check_all_free_list();
    enable_interrupt();
    DEBUG = 0;
}

void test_chunk()
{
    DEBUG = 1;
    disable_interrupt();
    uart_getc();
    uart_puts("\nallocate 0x1f\n");
    void *a = alloc_chunk(0x1f);
    uart_hex((unsigned long long)a);
    uart_puts("\n\n");

    for (int i=0; i<127; i++) {
        uart_hex((unsigned long long)alloc_chunk(0x1f));
        uart_puts("\t");
        uart_int(i);
        uart_puts("\n");
    }
    uart_puts("\n");
    
    uart_getc();
    uart_puts("\nallocate 0x1f\n");
    void *b = alloc_chunk(0x1f);
    uart_hex((unsigned long long)b);
    uart_puts("\n");


    void *ptr[64];

    uart_getc();
    uart_puts("\nallocate 0x40\n");
    ptr[0] = alloc_chunk(0x40);
    uart_hex((unsigned long long)ptr[0]);
    uart_puts("\n\n");

    for (int i=1; i<64; i++) {
        ptr[i] = alloc_chunk(0x40);
        uart_hex((unsigned long long)ptr[i]);
        uart_puts("\t");
        uart_int(i);
        uart_puts("\n");
    }

    uart_puts("\nallocate 0x40\n");
    void *c = alloc_chunk(0x40);
    uart_hex((unsigned long long)c);
    
    uart_puts("\nfree:\n");
    
    for (int i=0; i<64; i++) {
        free_chunk(ptr[i]);
    }
    uart_puts("\n");
    
    
    uart_getc();
    uart_puts("\nallocate 0x40\n");
    void *d = alloc_chunk(0x40);
    uart_hex((unsigned long long)d);
    uart_puts("\n");


    free_chunk(a);
    free_chunk(b);
    // free_chunk(c);
    free_chunk(d);
    enable_interrupt();
    DEBUG = 0;
}

void test_kmalloc()
{
    DEBUG = 1;
    disable_interrupt();
    void *chunk_ptr[5];
    void *page_ptr[11];
    for (int i = 0; i<5; i++)
    {
        uart_puts("kmalloc size:\t0x");
        uart_hex(CHUNK_SIZE << i);
        uart_puts("\n");
        enable_interrupt();
        chunk_ptr[i] = kmalloc(CHUNK_SIZE << i);
        disable_interrupt();
        uart_puts("address:\t");
        uart_hex((uint64_t)chunk_ptr[i]);
        uart_puts("\n\n");
    }
    uart_getc();
    for (int i = 0; i<11; i++)
    {
        uart_puts("kmalloc size:\t0x");
        uart_hex(PAGE_SIZE << i);
        uart_puts("\n");
        enable_interrupt();
        page_ptr[i] = kmalloc(PAGE_SIZE << i);
        disable_interrupt();
        uart_puts("address:\t");
        uart_hex((uint64_t)page_ptr[i]);
        uart_puts("\n\n");
    }

    uart_getc();
    for (int i = 0; i<5; i++)
    {
        uart_puts("kfree address:\t0x");
        uart_hex((uint64_t)chunk_ptr[i]);
        uart_puts("\n");
        enable_interrupt();
        kfree(chunk_ptr[i]);
        disable_interrupt();
    }

    uart_getc();
    for (int i = 0; i<11; i++)
    {
        uart_puts("kfree address:\t0x");
        uart_hex((uint64_t)page_ptr[i]);
        uart_puts("\n");
        enable_interrupt();
        kfree(page_ptr[i]);
        disable_interrupt();
    }
    enable_interrupt();
    DEBUG = 0;

}

void test_reserve()
{
    DEBUG = 1;
    memory_reserve(0x234, 0x4203);
    DEBUG = 0;
}