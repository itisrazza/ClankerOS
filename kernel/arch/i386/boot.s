/* boot.s - Multiboot boot stub for ClankerOS */

.set ALIGN,    1<<0             /* align loaded modules on page boundaries */
.set MEMINFO,  1<<1             /* provide memory map */
.set FLAGS,    ALIGN | MEMINFO  /* multiboot flags */
.set MAGIC,    0x1BADB002       /* multiboot magic number */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum required by multiboot */

/* Multiboot header */
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

/* Set up stack */
.section .bss
.align 16
stack_bottom:
.skip 16384 /* 16 KB stack */
stack_top:

/* Entry point */
.section .text
.global _start
.type _start, @function
_start:
    /* Set up the stack */
    mov $stack_top, %esp

    /* Save multiboot info pointer (EBX) and magic (EAX) */
    push %ebx
    push %eax

    /* Call kernel main function */
    call kernel_main

    /* If kernel_main returns, halt */
    cli
1:  hlt
    jmp 1b

.size _start, . - _start
