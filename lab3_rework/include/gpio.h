#ifndef _GPIO_H
#define _GPIO_H

#define MMIO_BASE       0x3F000000

#define GPFSEL0         ((volatile unsigned int*)(MMIO_BASE+0x00200000))
#define GPFSEL1         ((volatile unsigned int*)(MMIO_BASE+0x00200004))
#define GPFSEL2         ((volatile unsigned int*)(MMIO_BASE+0x00200008))
#define GPFSEL3         ((volatile unsigned int*)(MMIO_BASE+0x0020000C))
#define GPFSEL4         ((volatile unsigned int*)(MMIO_BASE+0x00200010))
#define GPFSEL5         ((volatile unsigned int*)(MMIO_BASE+0x00200014))

#define GPSET0          ((volatile unsigned int*)(MMIO_BASE+0x0020001C))
#define GPSET1          ((volatile unsigned int*)(MMIO_BASE+0x00200020))
#define GPCLR0          ((volatile unsigned int*)(MMIO_BASE+0x00200028))
#define GPCLR1          ((volatile unsigned int*)(MMIO_BASE+0x0020002c))
#define GPLEV0          ((volatile unsigned int*)(MMIO_BASE+0x00200034))
#define GPLEV1          ((volatile unsigned int*)(MMIO_BASE+0x00200038))
#define GPEDS0          ((volatile unsigned int*)(MMIO_BASE+0x00200040))
#define GPEDS1          ((volatile unsigned int*)(MMIO_BASE+0x00200044))
#define GPREN0          ((volatile unsigned int*)(MMIO_BASE+0x0020004c))
#define GPREN1          ((volatile unsigned int*)(MMIO_BASE+0x00200050))
#define GPFEN0          ((volatile unsigned int*)(MMIO_BASE+0x00200058))
#define GPFEN1          ((volatile unsigned int*)(MMIO_BASE+0x0020005c))
#define GPHEN0          ((volatile unsigned int*)(MMIO_BASE+0x00200064))
#define GPHEN1          ((volatile unsigned int*)(MMIO_BASE+0x00200068))
#define GPLEN0          ((volatile unsigned int*)(MMIO_BASE+0x00200070))
#define GPLEN1          ((volatile unsigned int*)(MMIO_BASE+0x00200074))
#define GPAREN0         ((volatile unsigned int*)(MMIO_BASE+0x0020007c))
#define GPAREN1         ((volatile unsigned int*)(MMIO_BASE+0x00200080))
#define GPAFEN0         ((volatile unsigned int*)(MMIO_BASE+0x00200088))
#define GPAFEN1         ((volatile unsigned int*)(MMIO_BASE+0x0020008c))
#define GPPUD           ((volatile unsigned int*)(MMIO_BASE+0x00200094))
#define GPPUDCLK0       ((volatile unsigned int*)(MMIO_BASE+0x00200098))
#define GPPUDCLK1       ((volatile unsigned int*)(MMIO_BASE+0x0020009C))



#define INTERRUPT_BASE          MMIO_BASE+0xb000

#define IRQ_BASIC_PEND          ((volatile unsigned int*)(INTERRUPT_BASE+0x200))
#define IRQ_PEND1               ((volatile unsigned int*)(INTERRUPT_BASE+0x204))
#define IRQ_PEND2               ((volatile unsigned int*)(INTERRUPT_BASE+0x208))
#define FIQ_CTRL                ((volatile unsigned int*)(INTERRUPT_BASE+0x20c))
#define ENABLE_IRQS1            ((volatile unsigned int*)(INTERRUPT_BASE+0x210))
#define ENABLE_IRQS2            ((volatile unsigned int*)(INTERRUPT_BASE+0x214))
#define ENABLE_BASIC_IRQS       ((volatile unsigned int*)(INTERRUPT_BASE+0x218))
#define DISABLE_IRQS1           ((volatile unsigned int*)(INTERRUPT_BASE+0x21c))
#define DISABLE_IRQS2           ((volatile unsigned int*)(INTERRUPT_BASE+0x220))
#define DISABLE_BASIC_IRQS      ((volatile unsigned int*)(INTERRUPT_BASE+0x224))

#endif