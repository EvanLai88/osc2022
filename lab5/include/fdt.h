#ifndef DTB_H
#define DTB_H

#include "compiler.h"

// manipulate device tree with dtb file format
#define FDT_BEGIN_NODE 0x00000001
#define FDT_END_NODE 0x00000002
#define FDT_PROP 0x00000003
#define FDT_NOP 0x00000004
#define FDT_END 0x00000009

extern void* CPIO_BASE;
extern void* CPIO_END;
typedef void (*dtb_callback)(uint32_t node_type, char *name, void *value, uint32_t name_size);

uint32_t uint32_big2little(uint32_t data);
void traverse_device_tree(void *base,dtb_callback callback);  //traverse dtb tree
void dtb_callback_show_tree(uint32_t node_type, char *name, void *value, uint32_t name_size);
void dtb_callback_initramfs(uint32_t node_type, char *name, void *value, uint32_t name_size);
void dtb_reserve_mem();
#endif