/* linker.ld - Linker script for ClankerOS kernel (i386) */

ENTRY(_start)

SECTIONS
{
    /* Kernel starts at 1MB physical address */
    . = 1M;

    /* Text (code) section - multiboot header must be at the start */
    .text ALIGN(4K) :
    {
        *(.multiboot)
        *(.text)
    }

    /* Read-only data section */
    .rodata ALIGN(4K) :
    {
        *(.rodata)
    }

    /* Initialized data section */
    .data ALIGN(4K) :
    {
        *(.data)
    }

    /* Uninitialized data section */
    .bss ALIGN(4K) :
    {
        *(COMMON)
        *(.bss)
    }

    /* Kernel end marker */
    _kernel_end = .;
}
