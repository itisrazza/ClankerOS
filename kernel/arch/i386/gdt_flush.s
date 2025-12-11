/* gdt_flush.s - Assembly code to load GDT */

.section .text

.global gdtFlush
.type gdtFlush, @function

gdtFlush:
    /* Load GDT pointer passed as argument */
    mov 4(%esp), %eax
    lgdt (%eax)

    /* Far jump to reload CS register */
    /* 0x08 is offset to kernel code segment */
    jmp $0x08, $flush2

flush2:
    /* Reload data segment registers */
    mov $0x10, %ax      /* 0x10 is offset to kernel data segment */
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    ret

.size gdtFlush, . - gdtFlush
