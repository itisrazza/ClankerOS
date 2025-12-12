/* panic.c - Kernel Panic Handler Implementation */

#include "panic.h"
#include "early_console.h"
#include <stdarg.h>

/* Forward declarations */
extern void VidWriteString(const char* str);
extern void VidPutChar(char c);

/* Simple helper to write string to serial */
static void serialWriteString(const char* str)
{
    while (*str) {
        EConPutChar(*str++);
    }
}

/* Simple hex to string conversion (no allocation) */
static void writeHex(char* buf, uint32_t value, int width)
{
    const char* hexDigits = "0123456789abcdef";
    for (int i = width - 1; i >= 0; i--) {
        buf[i] = hexDigits[value & 0xF];
        value >>= 4;
    }
    buf[width] = '\0';
}

/* Simple decimal to string conversion (no allocation) */
static void writeDecimal(char* buf, int value)
{
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    char temp[12];
    int i = 0;
    int isNegative = 0;

    if (value < 0) {
        isNegative = 1;
        value = -value;
    }

    while (value > 0) {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    }

    int j = 0;
    if (isNegative) {
        buf[j++] = '-';
    }

    while (i > 0) {
        buf[j++] = temp[--i];
    }
    buf[j] = '\0';
}

/*
 * KPanicImpl - Kernel panic implementation
 */
void KPanicImpl(const char* file, int line, const char* fmt, ...)
{
    char numBuf[12];

    // Disable interrupts - we're in a bad state
    __asm__ volatile ("cli");

    // Print panic header to both outputs
    VidWriteString("\n\n!!! KERNEL PANIC !!!\n");
    serialWriteString("\n\n");
    serialWriteString("================================================================================\n");
    serialWriteString("!!!                          KERNEL PANIC                                   !!!\n");
    serialWriteString("================================================================================\n");

    // Print source location
    VidWriteString("Location: ");
    VidWriteString(file);
    VidWriteString(":");
    writeDecimal(numBuf, line);
    VidWriteString(numBuf);
    VidWriteString("\n");

    serialWriteString("Location: ");
    serialWriteString(file);
    serialWriteString(":");
    serialWriteString(numBuf);
    serialWriteString("\n");

    // Print panic message (simple format string support - %s, %d, %x only)
    VidWriteString("Message: ");
    serialWriteString("Message: ");

    va_list args;
    va_start(args, fmt);

    const char* p = fmt;
    while (*p) {
        if (*p == '%' && *(p + 1)) {
            p++;
            // Skip width/precision specifiers (like 08 in %08x)
            while (*p >= '0' && *p <= '9') {
                p++;
            }
            // Now handle the actual format specifier
            if (*p == 's') {
                const char* str = va_arg(args, const char*);
                if (str) {
                    VidWriteString(str);
                    serialWriteString(str);
                } else {
                    VidWriteString("(null)");
                    serialWriteString("(null)");
                }
            } else if (*p == 'd' || *p == 'u') {
                int val = va_arg(args, int);
                writeDecimal(numBuf, val);
                VidWriteString(numBuf);
                serialWriteString(numBuf);
            } else if (*p == 'x') {
                uint32_t val = va_arg(args, uint32_t);
                writeHex(numBuf, val, 8);
                VidWriteString(numBuf);
                serialWriteString(numBuf);
            } else {
                VidPutChar(*p);
                EConPutChar(*p);
            }
            p++;
        } else {
            VidPutChar(*p);
            EConPutChar(*p);
            p++;
        }
    }

    va_end(args);

    VidWriteString("\n");
    serialWriteString("\n");

    // Print footer
    VidWriteString("\nSystem halted.\n");
    serialWriteString("\nSystem halted. CPU in halt state.\n");
    serialWriteString("================================================================================\n");

    // Halt forever
    while (1) {
        __asm__ volatile ("hlt");
    }
}

/*
 * KPanicWithRegsImpl - Kernel panic with register dump
 */
void KPanicWithRegsImpl(const char* file, int line, registers_t* regs,
                        const char* fmt, ...)
{
    char numBuf[12];

    // Disable interrupts
    __asm__ volatile ("cli");

    // Print panic header
    VidWriteString("\n\n!!! KERNEL PANIC !!!\n");
    serialWriteString("\n\n");
    serialWriteString("================================================================================\n");
    serialWriteString("!!!                          KERNEL PANIC                                   !!!\n");
    serialWriteString("================================================================================\n");

    // Print source location
    VidWriteString("Location: ");
    VidWriteString(file);
    VidWriteString(":");
    writeDecimal(numBuf, line);
    VidWriteString(numBuf);
    VidWriteString("\n");

    serialWriteString("Location: ");
    serialWriteString(file);
    serialWriteString(":");
    serialWriteString(numBuf);
    serialWriteString("\n");

    // Print panic message (simple format string support - %s, %d, %x only)
    VidWriteString("Message: ");
    serialWriteString("Message: ");

    va_list args;
    va_start(args, fmt);

    const char* p = fmt;
    while (*p) {
        if (*p == '%' && *(p + 1)) {
            p++;
            // Skip width/precision specifiers (like 08 in %08x)
            while (*p >= '0' && *p <= '9') {
                p++;
            }
            // Now handle the actual format specifier
            if (*p == 's') {
                const char* str = va_arg(args, const char*);
                if (str) {
                    VidWriteString(str);
                    serialWriteString(str);
                } else {
                    VidWriteString("(null)");
                    serialWriteString("(null)");
                }
            } else if (*p == 'd' || *p == 'u') {
                int val = va_arg(args, int);
                writeDecimal(numBuf, val);
                VidWriteString(numBuf);
                serialWriteString(numBuf);
            } else if (*p == 'x') {
                uint32_t val = va_arg(args, uint32_t);
                writeHex(numBuf, val, 8);
                VidWriteString(numBuf);
                serialWriteString(numBuf);
            } else {
                VidPutChar(*p);
                EConPutChar(*p);
            }
            p++;
        } else {
            VidPutChar(*p);
            EConPutChar(*p);
            p++;
        }
    }

    va_end(args);

    VidWriteString("\n");
    serialWriteString("\n");

    // Print register dump to serial (VGA output would be too much)
    if (regs) {
        serialWriteString("\nCPU Register Dump:\n");

        serialWriteString("  EIP: 0x");
        writeHex(numBuf, regs->eip, 8);
        serialWriteString(numBuf);
        serialWriteString("  CS:  0x");
        writeHex(numBuf, regs->cs, 4);
        serialWriteString(numBuf);
        serialWriteString("  EFLAGS: 0x");
        writeHex(numBuf, regs->eflags, 8);
        serialWriteString(numBuf);
        serialWriteString("\n");

        serialWriteString("  EAX: 0x");
        writeHex(numBuf, regs->eax, 8);
        serialWriteString(numBuf);
        serialWriteString("  EBX: 0x");
        writeHex(numBuf, regs->ebx, 8);
        serialWriteString(numBuf);
        serialWriteString("  ECX: 0x");
        writeHex(numBuf, regs->ecx, 8);
        serialWriteString(numBuf);
        serialWriteString("  EDX: 0x");
        writeHex(numBuf, regs->edx, 8);
        serialWriteString(numBuf);
        serialWriteString("\n");

        serialWriteString("  ESP: 0x");
        writeHex(numBuf, regs->esp, 8);
        serialWriteString(numBuf);
        serialWriteString("  EBP: 0x");
        writeHex(numBuf, regs->ebp, 8);
        serialWriteString(numBuf);
        serialWriteString("  ESI: 0x");
        writeHex(numBuf, regs->esi, 8);
        serialWriteString(numBuf);
        serialWriteString("  EDI: 0x");
        writeHex(numBuf, regs->edi, 8);
        serialWriteString(numBuf);
        serialWriteString("\n");

        serialWriteString("  DS:  0x");
        writeHex(numBuf, regs->ds, 4);
        serialWriteString(numBuf);
        serialWriteString("  SS:  0x");
        writeHex(numBuf, regs->ss, 4);
        serialWriteString(numBuf);
        serialWriteString("\n");

        serialWriteString("  INT: ");
        writeDecimal(numBuf, regs->intNo);
        serialWriteString(numBuf);
        serialWriteString("  ERR: 0x");
        writeHex(numBuf, regs->errCode, 8);
        serialWriteString(numBuf);
        serialWriteString("\n");

        // Print brief register info to VGA
        VidWriteString("EIP: 0x");
        writeHex(numBuf, regs->eip, 8);
        VidWriteString(numBuf);
        VidWriteString("  ESP: 0x");
        writeHex(numBuf, regs->esp, 8);
        VidWriteString(numBuf);
        VidWriteString("\n");
        VidWriteString("(See serial for full dump)\n");
    }

    // Print footer
    VidWriteString("\nSystem halted.\n");
    serialWriteString("\nSystem halted. CPU in halt state.\n");
    serialWriteString("================================================================================\n");

    // Halt forever
    while (1) {
        __asm__ volatile ("hlt");
    }
}
