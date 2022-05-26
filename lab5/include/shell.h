#ifndef _SHELL_H
#define _SHELL_H

#include "compiler.h"

void shell();
void shell_prompt();
void help();
void hello_world();
void reboot();
void cancel_reboot();
void ls();
void cat(char *filename);
void exefile(void *filename);
void test_page();
void test_chunk();
void test_kmalloc();
void test_reserve();

#endif