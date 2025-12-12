/* irq_stubs.s - Assembly stubs for hardware interrupt requests */

.section .text

/* External C handler */
.extern irqHandler

/* Macro for IRQ stubs that don't push an error code */
.macro IRQ num, intNum
.global irq\num
irq\num:
    cli
    pushl $0                    /* Dummy error code */
    pushl $\intNum              /* Interrupt number */
    jmp irqCommonStub
.endm

/* Define all 16 IRQ handlers (IRQ 0-15 map to interrupts 32-47) */
IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

/* Common IRQ stub - saves state and calls C handler */
irqCommonStub:
    /* Save all registers */
    pushl %eax
    pushl %ecx
    pushl %edx
    pushl %ebx
    pushl %esp
    pushl %ebp
    pushl %esi
    pushl %edi

    /* Call C handler */
    call irqHandler

    /* Restore registers */
    popl %edi
    popl %esi
    popl %ebp
    popl %esp
    popl %ebx
    popl %edx
    popl %ecx
    popl %eax

    /* Clean up error code and interrupt number */
    addl $8, %esp

    /* Return from interrupt */
    sti
    iret
