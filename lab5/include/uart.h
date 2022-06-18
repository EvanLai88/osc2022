#ifndef _UART_H
#define _UART_H

#include "compiler.h"
#include "gpio.h"

#define BUFFER_SIZE     0x100

#define ECHO_OFF        0x000
#define ECHO            0x001

#define NO_NEW_LINE     0x000
#define NEW_LINE        0x001

/* Auxilary mini UART registers */
#define AUX_ENABLE      ((volatile unsigned int*)(MMIO_BASE+0x00215004))
#define AUX_MU_IO       ((volatile unsigned int*)(MMIO_BASE+0x00215040))
#define AUX_MU_IER      ((volatile unsigned int*)(MMIO_BASE+0x00215044))
#define AUX_MU_IIR      ((volatile unsigned int*)(MMIO_BASE+0x00215048))
#define AUX_MU_LCR      ((volatile unsigned int*)(MMIO_BASE+0x0021504C))
#define AUX_MU_MCR      ((volatile unsigned int*)(MMIO_BASE+0x00215050))
#define AUX_MU_LSR      ((volatile unsigned int*)(MMIO_BASE+0x00215054))
#define AUX_MU_MSR      ((volatile unsigned int*)(MMIO_BASE+0x00215058))
#define AUX_MU_SCRATCH  ((volatile unsigned int*)(MMIO_BASE+0x0021505C))
#define AUX_MU_CNTL     ((volatile unsigned int*)(MMIO_BASE+0x00215060))
#define AUX_MU_STAT     ((volatile unsigned int*)(MMIO_BASE+0x00215064))
#define AUX_MU_BAUD     ((volatile unsigned int*)(MMIO_BASE+0x00215068))

void  uart_init();
void enable_mini_uart_interrupt();
void disable_mini_uart_interrupt();
void enable_mini_uart_r_interrupt();
void disable_mini_uart_r_interrupt();
void enable_mini_uart_w_interrupt();
void disable_mini_uart_w_interrupt();

void uart_disable_echo();
void uart_enable_echo();

void  uart_send(unsigned int c);
char  uart_getc();

void uart_async_putc(char c);
char uart_async_getc();
void uart_interrupt_r_handler();
void uart_interrupt_w_handler();

void  echo(char r);

char* uart_gets(char* buffer);
void  uart_puts(char *s);
void  uart_putln(char *s);
void  uart_puts_const(const char *s);
void  uart_puts_len(char *s, unsigned long len);
void  uart_hex(unsigned long long d);
int   uart_int(int d);

char* uart_async_gets(char *buf);
void uart_async_puts(char *s);
void uart_async_puts_const(const char *s);
void uart_async_puts_len(char *s, unsigned long len);
void uart_async_hex(unsigned long long d);
int uart_async_int(int d);

void  uart_printf();
#endif