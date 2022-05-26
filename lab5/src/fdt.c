#include "fdt.h"
#include "lib.h"
#include "uart.h"
#include "malloc.h"

char* DTB_PLACE;

//stored as big endian
struct fdt_header {
    uint32_t magic;
    uint32_t totalsize;
    uint32_t off_dt_struct;
    uint32_t off_dt_strings;
    uint32_t off_mem_rsvmap;
    uint32_t version;
    uint32_t last_comp_version;
    uint32_t boot_cpuid_phys;
    uint32_t size_dt_strings;
    uint32_t size_dt_struct;
};

struct fdt_reserve_entry {
    uint64_t address;
    uint64_t size;
};


uint32_t uint32_big2little(uint32_t data)
{
    char* r = (char*)&data;
    return (r[3]<<0) | (r[2]<<8) | (r[1]<<16) | (r[0]<<24);
}

uint64_t uint64_big2little(uint64_t data)
{
    char* r = (char*)&data;
    return ((uint64_t)r[7] << 0) | ((uint64_t)r[6] << 8) | ((uint64_t)r[5] << 16) | ((uint64_t)r[4] << 24) | ((uint64_t)r[3] << 32) | ((uint64_t)r[2] << 40) | ((uint64_t)r[1] << 48) | ((uint64_t)r[0] << 56);
}

void traverse_device_tree(void *dtb_ptr,dtb_callback callback)
{
    struct fdt_header* header = dtb_ptr;
    if(uint32_big2little(header->magic) != 0xD00DFEED)
    {
        uart_async_puts("traverse_device_tree : wrong magic in traverse_device_tree");
        return;
    }

    uint32_t struct_size = uint32_big2little(header->size_dt_struct);
    char* dt_struct_ptr = (char*)((char*)header + uint32_big2little(header->off_dt_struct));
    char* dt_strings_ptr = (char*)((char*)header + uint32_big2little(header->off_dt_strings));

    char* end = (char*)dt_struct_ptr + struct_size;
    char* pointer = dt_struct_ptr;

    while(pointer < end)
    {
        uint32_t token_type = uint32_big2little(*(uint32_t*)pointer);

        pointer += 4;
        if(token_type == FDT_BEGIN_NODE)
        { 
            callback(token_type,pointer,0,0);
            pointer += len(pointer);
            pointer += 4 - (unsigned long long)pointer%4;           //alignment 4 byte
        }else if(token_type == FDT_END_NODE)
        {
            callback(token_type,0,0,0);
        }else if(token_type == FDT_PROP)
        {
            uint32_t len = uint32_big2little(*(uint32_t*)pointer);
            pointer += 4;
            char* name = (char*)dt_strings_ptr + uint32_big2little(*(uint32_t*)pointer);
            pointer += 4;
            callback(token_type,name,pointer,len);
            pointer += len;
            if((unsigned long long)pointer % 4 !=0)pointer += 4 - (unsigned long long)pointer%4;   //alignment 4 byte
        }else if(token_type == FDT_NOP)
        {
            callback(token_type,0,0,0);
        }else if(token_type == FDT_END)
        {
            callback(token_type,0,0,0);
        }else
        {
            uart_async_puts("error type:");
            uart_async_hex(token_type);
            uart_async_puts("\n");
            return;
        }
    }
}

void dtb_callback_show_tree(uint32_t node_type, char *name, void *data, uint32_t data_size)
{
    static int level = 0;
    if(node_type==FDT_BEGIN_NODE)
    {
        for(int i=0;i<level;i++)uart_async_puts("   ");
        uart_async_puts(name);
        uart_async_puts(": {\n");
        level++;
    }else if(node_type==FDT_END_NODE)
    {
        level--;
        for(int i=0;i<level;i++)uart_async_puts("   ");
        uart_async_puts("}\n");
    }else if(node_type==FDT_PROP)
    {
        for(int i=0;i<level;i++)uart_async_puts("   ");
        uart_async_puts(name);
        uart_async_puts(": ");
        // if (data_size == 4) {
        //     uart_async_hex((unsigned long long)data);
        // }
        // else {
        //     uart_async_puts(data);
        // }
        uart_async_puts(data);
        uart_async_puts("\n");
    }
}

void dtb_callback_initramfs(uint32_t node_type, char *name, void *value, uint32_t value_size) {
    if(node_type==FDT_PROP && strcmp(name,"linux,initrd-start")==0)
    {
        CPIO_BASE = (void *)(uint64_t)uint32_big2little(*(uint32_t*)value);
    }

    if(node_type==FDT_PROP && strcmp(name,"linux,initrd-end")==0)
    {
        CPIO_END = (void *)(uint64_t)uint32_big2little(*(uint32_t *)value);
    }
    
}

void dtb_reserve_mem()
{
    struct fdt_header *header = (struct fdt_header *) DTB_PLACE;
    uart_puts("DTB_reserve:\n");
    memory_reserve((uint64_t)DTB_PLACE, (uint64_t)DTB_PLACE + uint32_big2little(header->totalsize));

    if (uint32_big2little(header->magic) != 0xD00DFEED)
    {
        uart_puts("traverse_device_tree : wrong magic in traverse_device_tree");
        return;
    }

    struct fdt_reserve_entry *rsv_entry = (struct fdt_reserve_entry *)((void *)header + uint32_big2little(header->off_mem_rsvmap));
    while(rsv_entry->address != 0 || rsv_entry->size != 0)
    {
        uint64_t start = uint64_big2little(rsv_entry->address);
        uint64_t end = start + uint64_big2little(rsv_entry->size);
        uart_puts("dtb_rsv_entry:\n");
        // uart_async_puts("\tstart:\t");
        // uart_async_hex(start);
        // uart_async_puts("\n\tend:\t");
        // uart_async_hex(end);
        // uart_async_puts("\n");
        memory_reserve(start, end);
        rsv_entry++;
    }
}