#include "uart.h"
#include "exception.h"

int state = 0;
int count, left_count = 0;

char Tx_buf[BUFFER_SIZE] = {};
unsigned int Tx_buf_r_idx = 0;
unsigned int Tx_buf_w_idx = 0;

char Rx_buf[BUFFER_SIZE] = {};
unsigned int Rx_buf_r_idx = 0;
unsigned int Rx_buf_w_idx = 0;

/**
 * Set baud rate and characteristics (115200 8N1) and map to GPIO
 */
void uart_init()
{
    register unsigned int r;

    /* initialize UART */
    *AUX_ENABLE |=1;       // enable UART1, AUX mini uart
    *AUX_MU_CNTL = 0;      // Disable Tx and Rx during configuration
    *AUX_MU_IER = 0;       // Disable interrupt
    *AUX_MU_LCR = 3;       // Set the data size to 8 bit
    *AUX_MU_MCR = 0;       // Don’t need auto flow control
    *AUX_MU_BAUD = 270;    // Set baud rate to 115200, 250M/(8*271) = 115,313.6531365314
    *AUX_MU_IIR = 0x6;    // clear FIFO's transmit and receive 0b110

    /* map UART1 to GPIO pins */
    r = *GPFSEL1;
    r &= ~((7<<12)|(7<<15)); // zero fill gpio14, gpio15, bit position
    r |= (2<<12)|(2<<15);    // set alt5 on gpio14, gpio15
    *GPFSEL1 = r;
    *GPPUD = 0;              // disable pull-up/down
    r=150; while(r--) { asm volatile("nop"); }          // wait 150 cycles for gpio setup control signal
    *GPPUDCLK0 = (1<<14)|(1<<15);                       // assert clock control signal on pin 14, 15
    r=150; while(r--) { asm volatile("nop"); }          // wait 150 cycles
    *GPPUDCLK0 = 0;        // flush GPIO setup          // remove clock
    
    *AUX_MU_CNTL = 3;      // enable Tx, Rx 

    while((*AUX_MU_LSR&0x01))*AUX_MU_IO; // clear Rx
}

void enable_mini_uart_interrupt()
{
    *ENABLE_IRQS1 |= 1 << 29;
}

void disable_mini_uart_interrupt()
{
    *ENABLE_IRQS1 |= 1 << 29;
    *ENABLE_IRQS1 ^= 1 << 29;
}

void enable_mini_uart_r_interrupt()
{
    *AUX_MU_IER |= 1;
}

void disable_mini_uart_r_interrupt()
{
    *AUX_MU_IER &= ~(1);
}

void enable_mini_uart_w_interrupt()
{
    *AUX_MU_IER |= 2;
}

void disable_mini_uart_w_interrupt()
{
    *AUX_MU_IER &= ~(2);
}

void  uart_send(unsigned int c) {
    do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x20));
    *AUX_MU_IO=c;
}

char uart_getc(int e) {
    char r;
    do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x01));
    r=(char)(*AUX_MU_IO);
    if (e == ECHO) {
        echo(r);
    }
    if (e == ECHO_OFF) {
        return r;
    }
    return r=='\r'?'\n':r;
}


void uart_async_putc(char c)
{
    while ((Tx_buf_w_idx + 1) % BUFFER_SIZE == Tx_buf_r_idx)
        enable_mini_uart_w_interrupt();

    // critical section
    disable_interrupt();
    Tx_buf[Tx_buf_w_idx++] = c;
    if (Tx_buf_w_idx >= BUFFER_SIZE)
        Tx_buf_w_idx = 0;
    enable_interrupt();
    enable_mini_uart_w_interrupt();
}

char uart_async_getc()
{
    enable_mini_uart_r_interrupt();
    while (Rx_buf_r_idx == Rx_buf_w_idx)
        enable_mini_uart_r_interrupt();

    // critical section
    disable_interrupt();
    char r = Rx_buf[Rx_buf_r_idx++];
    if (Rx_buf_r_idx >= BUFFER_SIZE)
        Rx_buf_r_idx = 0;
    enable_interrupt();
    return r;
}

void uart_interrupt_r_handler()
{
    //read buffer full
    if ((Rx_buf_w_idx + 1) % BUFFER_SIZE == Rx_buf_r_idx)
    {
        disable_mini_uart_r_interrupt(); //disable read interrupt when read buffer full
        return;
    }
    Rx_buf[Rx_buf_w_idx++] = uart_getc(ECHO);
    if (Rx_buf_w_idx >= BUFFER_SIZE)
        Rx_buf_w_idx = 0;

    enable_mini_uart_r_interrupt(); // lab 3 : advanced 2 -> unmask device line
}

void uart_interrupt_w_handler() //can write
{
    // buffer empty
    if (Tx_buf_r_idx == Tx_buf_w_idx)
    {
        disable_mini_uart_w_interrupt(); // disable w_interrupt to prevent interruption without any async output
        return;
    }
    uart_send(Tx_buf[Tx_buf_r_idx++]);
    if (Tx_buf_r_idx >= BUFFER_SIZE)
        Tx_buf_r_idx = 0; // cycle pointer

    enable_mini_uart_w_interrupt(); // lab 3 : advanced 2 -> unmask device line
}

void echo(char r) {
    if (r == '\r'){
        uart_send('\r');
        uart_send('\n');
    }
    else if (r == '\x7f') {
        uart_send('\b');
        uart_send(' ');
        uart_send('\b');
    }
    else if (r == '\x1b') {

    }
    else if (r == '\x5b') {
        
    }
    else if (state == 2) {
        
    }
    else if (r == '\x01') {

    }
    else if (r == '\x05') {

    }
    else {
        uart_send(r);
    }

}

/**
 * 0000001B 000005B 00000041         ^      "\033\133\101"  "\x1b\x5b\x41"
 * 0000001B 000005B 00000042         v      "\033\133\102"  "\x1b\x5b\x42"
 * 0000001B 000005B 00000043         >      "\033\133\103"  "\x1b\x5b\x43"
 * 0000001B 000005B 00000044         <      "\033\133\104"  "\x1b\x5b\x44"
 */
char* uart_gets(char *buf) {
    char c;
    char *s;

    count = 0;
    left_count = 0;
    state = 0;
    for (s = buf,count = 0; (c = uart_getc(ECHO)) != '\n' && count!=BUFFER_SIZE-1 ;count++) {
        
        if(state == 0) {
            
            if (left_count > 0) {
                count--;
                left_count--;
            }
            /* back space */
            if(c=='\x7f') {
                count--;                    // does not count as a char
                if(count==-1) {
                    uart_send(' ');         // recover " " from "# "
                    continue;
                }
                s--;                        // move curser left one char
                count--;                    // remove one char
                continue;
            }

            /* ESC */
            if(c=='\x1b') {
                count--;                    // does not count as a char
                state = 1;
                continue;
            }
        }
        else if (state == 1) {

            /* [ */
            if(c=='\x5b') {
                count--;                    // does not count as a char
                state = 2;
                continue;
            }

            state = 0;
        }
        else if (state == 2) {
            state = 0;                      // reset state

            /* up arrow */
            if(c=='A') {
                count--;                    // does not count as a char
                continue;
            }

            /* down arrow */
            if(c=='B') {
                count--;                    // does not count as a char
                continue;
            }

            /* right arrow */
            if(c=='C') {
                count--;                    // does not count as a char
                if (left_count > 0) {       // check border
                    s++;
                    left_count--;
                    uart_send('\x1b');
                    uart_send('[');
                    uart_send('C');
                }
                continue;
            }

            /* left arrow */
            if(c=='D') {
                count--;                    // does not count as a char
                if (left_count <= count) {   // check border
                    s--;
                    left_count++;
                    uart_send('\x1b');
                    uart_send('[');
                    uart_send('D');
                }
                continue;
            }
        }

        /* safe to write back */
        *s = c;
        s++;
    }

    for (int i=0; i<left_count; i++) {
        s++;
    }
	*s = '\0';
	return buf;
}

void uart_puts(char *s) {
    while(*s) {
        if(*s=='\n')
            uart_send('\r');
        uart_send(*s);
        s++;
    }
}

void uart_putln(char *s) {
    uart_puts(s);
    uart_puts("\n");
}

void uart_puts_const(const char *s) {
    while(*s) {
        if(*s=='\n')
            uart_send('\r');
        uart_send(*s);
        s++;
    }
}

void uart_puts_len(char *s, unsigned long len) {
    while(len) {
        if(*s=='\n')
            uart_send('\r');
        uart_send(*s);
        s++;
        len--;
    }
}

void uart_hex(unsigned long long d) {
    unsigned long long n;
    int c;
    for(c=60;c>=0;c-=4) {
        // get highest tetrad
        n=(d>>c)&0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n+=n>9?0x37:0x30;
        uart_send(n);
    }
}

int uart_int(int d) {
    char buf[BUFFER_SIZE];
    char tmp;
    int digits = 0, n = d;
    
    if (d == 0){
        uart_send('0');
        return 0;
    }

    if (n < 0) {
        n = -n;
    }

    while (n != 0) {
        digits++;
        n /= 10;
    }

    n = d;
    if (n < 0) {
        n = -n;
    }

    if ( digits > BUFFER_SIZE - 1) {
        uart_puts("Exceed buffer size.\n");
        return -1;
    }
    
    buf[digits] = '\0';
    for (int i = digits-1; i >= 0; i--) {
        tmp = n%10 + '0';
        n /= 10;
        buf[i] = tmp;
    }
    if (d<0) {
        uart_send('-');
    }
    uart_puts(buf);
    return 0;
}

char* uart_async_gets(char *buf) {
    char c;
    char *s;

    count = 0;
    left_count = 0;
    state = 0;
    for (s = buf,count = 0; (c = uart_async_getc()) != '\n' && count!=BUFFER_SIZE-1 ;count++) {
        
        if(state == 0) {
            
            if (left_count > 0) {
                count--;
                left_count--;
            }
            /* back space */
            if(c=='\x7f') {
                count--;                    // does not count as a char
                if(count==-1) {
                    uart_async_getc(' ');         // recover " " from "# "
                    continue;
                }
                s--;                        // move curser left one char
                count--;                    // remove one char
                continue;
            }

            /* ESC */
            if(c=='\x1b') {
                count--;                    // does not count as a char
                state = 1;
                continue;
            }
        }
        else if (state == 1) {

            /* [ */
            if(c=='\x5b') {
                count--;                    // does not count as a char
                state = 2;
                continue;
            }

            state = 0;
        }
        else if (state == 2) {
            state = 0;                      // reset state

            /* up arrow */
            if(c=='A') {
                count--;                    // does not count as a char
                continue;
            }

            /* down arrow */
            if(c=='B') {
                count--;                    // does not count as a char
                continue;
            }

            /* right arrow */
            if(c=='C') {
                count--;                    // does not count as a char
                if (left_count > 0) {       // check border
                    s++;
                    left_count--;
                    uart_async_getc('\x1b');
                    uart_async_getc('[');
                    uart_async_getc('C');
                }
                continue;
            }

            /* left arrow */
            if(c=='D') {
                count--;                    // does not count as a char
                if (left_count <= count) {   // check border
                    s--;
                    left_count++;
                    uart_async_getc('\x1b');
                    uart_async_getc('[');
                    uart_async_getc('D');
                }
                continue;
            }
        }

        /* safe to write back */
        *s = c;
        s++;
    }

    for (int i=0; i<left_count; i++) {
        s++;
    }
	*s = '\0';
	return buf;
}

void uart_async_puts(char *s) {
    while(*s) {
        if(*s=='\n')
            uart_async_putc('\r');
        uart_async_putc(*s);
        s++;
    }
}

void uart_async_puts_const(const char *s) {
    while(*s) {
        if(*s=='\n')
            uart_async_putc('\r');
        uart_async_putc(*s);
        s++;
    }
}

void uart_async_puts_len(char *s, unsigned long len) {
    while(len) {
        if(*s=='\n')
            uart_async_putc('\r');
        uart_async_putc(*s);
        s++;
        len--;
    }
}

void uart_async_hex(unsigned long long d) {
    unsigned long long n;
    int c;
    for(c=28;c>=0;c-=4) {
        // get highest tetrad
        n=(d>>c)&0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n+=n>9?0x37:0x30;
        uart_async_putc(n);
    }
}

int uart_async_int(int d) {
    char buf[BUFFER_SIZE];
    char tmp;
    int digits = 0, n = d;
    
    if (d == 0){
        uart_async_putc('0');
        return 0;
    }

    if (n < 0) {
        n = -n;
    }

    while (n != 0) {
        digits++;
        n /= 10;
    }

    n = d;
    if (n < 0) {
        n = -n;
    }

    if ( digits > BUFFER_SIZE - 1) {
        uart_puts("Exceed buffer size.\n");
        return -1;
    }
    
    buf[digits] = '\0';
    for (int i = digits-1; i >= 0; i--) {
        tmp = n%10 + '0';
        n /= 10;
        buf[i] = tmp;
    }
    if (d<0) {
        uart_async_putc('-');
    }
    uart_puts(buf);
    return 0;
}

void uart_printf(){

}