/* linker.ld - Linker script for ClankerOS kernel (i386) */

ENTRY(_start)

SECTIONS
{
    /* Kernel starts at 1MB physical address */
    . = 1M;

    /* Multiboot header must be in first 8KB */
    .multiboot :
    {
        *(.multiboot)
    }

    /* Text (code) section */
    .text ALIGN(4K) :
    {
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
