/* isr_stubs.s - ISR stub routines */

.section .text

/* Macro for ISRs that don't push an error code */
.macro ISR_NOERRCODE num
.global isr\num
isr\num:
    cli
    push $0                 /* Push dummy error code */
    push $\num              /* Push interrupt number */
    jmp isr_common_stub
.endm

/* Macro for ISRs that push an error code */
.macro ISR_ERRCODE num
.global isr\num
isr\num:
    cli
    push $\num              /* Push interrupt number (error already pushed) */
    jmp isr_common_stub
.endm

/* CPU Exception ISRs (0-31) */
ISR_NOERRCODE 0     /* Division By Zero */
ISR_NOERRCODE 1     /* Debug */
ISR_NOERRCODE 2     /* Non Maskable Interrupt */
ISR_NOERRCODE 3     /* Breakpoint */
ISR_NOERRCODE 4     /* Into Detected Overflow */
ISR_NOERRCODE 5     /* Out of Bounds */
ISR_NOERRCODE 6     /* Invalid Opcode */
ISR_NOERRCODE 7     /* No Coprocessor */
ISR_ERRCODE   8     /* Double Fault */
ISR_NOERRCODE 9     /* Coprocessor Segment Overrun */
ISR_ERRCODE   10    /* Bad TSS */
ISR_ERRCODE   11    /* Segment Not Present */
ISR_ERRCODE   12    /* Stack Fault */
ISR_ERRCODE   13    /* General Protection Fault */
ISR_ERRCODE   14    /* Page Fault */
ISR_NOERRCODE 15    /* Reserved */
ISR_NOERRCODE 16    /* Coprocessor Fault */
ISR_ERRCODE   17    /* Alignment Check */
ISR_NOERRCODE 18    /* Machine Check */
ISR_NOERRCODE 19    /* Reserved */
ISR_NOERRCODE 20    /* Reserved */
ISR_NOERRCODE 21    /* Reserved */
ISR_NOERRCODE 22    /* Reserved */
ISR_NOERRCODE 23    /* Reserved */
ISR_NOERRCODE 24    /* Reserved */
ISR_NOERRCODE 25    /* Reserved */
ISR_NOERRCODE 26    /* Reserved */
ISR_NOERRCODE 27    /* Reserved */
ISR_NOERRCODE 28    /* Reserved */
ISR_NOERRCODE 29    /* Reserved */
ISR_NOERRCODE 30    /* Reserved */
ISR_NOERRCODE 31    /* Reserved */

/* Common ISR stub - saves state and calls C handler */
isr_common_stub:
    /* Save all registers */
    pusha

    /* Save data segment */
    mov %ds, %ax
    push %eax

    /* Load kernel data segment */
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    /* Call C handler (registers_t* on stack) */
    push %esp
    call isrHandler
    add $4, %esp

    /* Restore data segment */
    pop %eax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    /* Restore registers */
    popa

    /* Clean up error code and interrupt number */
    add $8, %esp

    /* Return from interrupt */
    sti
    iret
