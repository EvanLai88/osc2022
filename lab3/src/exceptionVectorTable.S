// save general registers to stack
.macro save_all
    sub sp, sp, 32 * 12
    stp x0, x1, [sp ,16 * 0]
    stp x2, x3, [sp ,16 * 1]
    stp x4, x5, [sp ,16 * 2]
    stp x6, x7, [sp ,16 * 3]
    stp x8, x9, [sp ,16 * 4]
    stp x10, x11, [sp ,16 * 5]
    stp x12, x13, [sp ,16 * 6]
    stp x14, x15, [sp ,16 * 7]
    stp x16, x17, [sp ,16 * 8]
    stp x18, x19, [sp ,16 * 9]
    stp x20, x21, [sp ,16 * 10]
    stp x22, x23, [sp ,16 * 11]
    stp x24, x25, [sp ,16 * 12]
    stp x26, x27, [sp ,16 * 13]
    stp x28, x29, [sp ,16 * 14]
    str x30, [sp, 16 * 15]

    //using for nested interrupt
    mrs x0,  spsr_el1
    str x0,  [sp, 16 * 16]
    mrs x0,  elr_el1
    str x0,  [sp, 16 * 17]
    //mrs x0,  sp_el0
    //str x0,  [sp, 16 * 18]
    //mrs x0,  sp_el1
    //str x0,  [sp, 16 * 19]
    ldp x0, x1, [sp ,16 * 0]  // restore x0
.endm

// load general registers from stack
.macro load_all
    ldp x0, x1, [sp ,16 * 0]
    ldp x2, x3, [sp ,16 * 1]
    ldp x4, x5, [sp ,16 * 2]
    ldp x6, x7, [sp ,16 * 3]
    ldp x8, x9, [sp ,16 * 4]
    ldp x10, x11, [sp ,16 * 5]
    ldp x12, x13, [sp ,16 * 6]
    ldp x14, x15, [sp ,16 * 7]
    ldp x16, x17, [sp ,16 * 8]
    ldp x18, x19, [sp ,16 * 9]
    ldp x20, x21, [sp ,16 * 10]
    ldp x22, x23, [sp ,16 * 11]
    ldp x24, x25, [sp ,16 * 12]
    ldp x26, x27, [sp ,16 * 13]
    ldp x28, x29, [sp ,16 * 14]
    ldr x30, [sp, 16 * 15]

    //using for nested interrupt
    ldr x0, [sp, 16 * 16]
    msr spsr_el1,x0
    ldr x0,  [sp, 16 * 17]
    msr elr_el1, x0
    //ldr x0,  [sp, 16 * 18]
    //msr sp_el0, x0
    //ldr x0,  [sp, 16 * 19]
    //msr sp_el1, x0

    ldp x0, x1, [sp ,16 * 0]  // restore x0
    
    add sp, sp, 32 * 12
.endm

.macro    ventry    label
    .align    7
    b    \label
.endm

.align 11 // vector table should be aligned to 0x800
.global exception_vector_table
exception_vector_table:

    //Exception from the current EL while using SP_EL0
    ventry	sync_invalid_el1t			// Synchronous EL1t
    ventry	irq_invalid_el1t			// IRQ EL1t
    ventry	fiq_invalid_el1t			// FIQ EL1t
    ventry	error_invalid_el1t			// Error EL1t
    
    //Exception from the current EL while using SP_ELx
    ventry	sync_el1h			        // Synchronous EL1h	
    ventry	irq_el1h		        	// IRQ EL1h	
    ventry	fiq_invalid_el1h			// FIQ EL1h
    ventry	error_invalid_el1h			// Error EL1h

    //Exception from a lower EL and at least one lower EL is AArch64
    ventry	sync_el0_64			        // Synchronous 64-bit EL0
    ventry	irq_el0_64			        // IRQ 64-bit EL0
    ventry	fiq_invalid_el0_64			// FIQ 64-bit EL0
    ventry	error_invalid_el0_64		// Error 64-bit EL0
        
    //Exception from a lower EL and at least all lower EL are AArch32
    ventry	sync_invalid_el0_32			// Synchronous 32-bit EL0
    ventry	irq_invalid_el0_32			// IRQ 32-bit EL0
    ventry	fiq_invalid_el0_32			// FIQ 32-bit EL0
    ventry	error_invalid_el0_32			// Error 32-bit EL0



sync_invalid_el1t:
    save_all
    mov x0,0
    bl invalid_exception_router
    load_all
    eret
irq_invalid_el1t:
    save_all
    mov x0,1
    bl invalid_exception_router
    load_all
    eret
fiq_invalid_el1t:
    save_all
    mov x0,2
    bl invalid_exception_router
    load_all
    eret
error_invalid_el1t:
    save_all
    mov x0,3
    bl invalid_exception_router
    load_all
    eret
        


sync_el1h:
    save_all
    mov x0,4
    bl invalid_exception_router
    load_all
    eret
irq_el1h:
    save_all
    mov x0,5
    bl irq_router
    load_all
    eret
fiq_invalid_el1h:
    save_all
    mov x0,6
    bl invalid_exception_router
    load_all
    eret
error_invalid_el1h:
    save_all
    mov x0,7
    bl invalid_exception_router
    load_all
    eret



sync_el0_64:
    save_all
    mov x0,8
    bl sync_64_router
    load_all
    eret
irq_el0_64:
    save_all
    mov x0,9
    bl irq_router
    load_all
    eret
fiq_invalid_el0_64:
    save_all
    mov x0,10
    bl invalid_exception_router
    load_all
    eret
error_invalid_el0_64:
    save_all
    mov x0,11
    bl invalid_exception_router
    load_all
    eret



sync_invalid_el0_32:
    save_all
    mov x0,12
    bl invalid_exception_router
    load_all
    eret
irq_invalid_el0_32:
    save_all
    mov x0,13
    bl invalid_exception_router
    load_all
    eret
fiq_invalid_el0_32:
    save_all
    mov x0,14
    bl invalid_exception_router
    load_all
    eret
error_invalid_el0_32:
    save_all
    mov x0,15
    bl invalid_exception_router
    load_all
    eret