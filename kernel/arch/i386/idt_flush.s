/* idt_flush.s - Assembly code to load IDT */

.section .text
.global idtFlush
.type idtFlush, @function

idtFlush:
    /* Load IDT pointer passed as argument */
    mov 4(%esp), %eax
    lidt (%eax)
    ret

.size idtFlush, . - idtFlush
