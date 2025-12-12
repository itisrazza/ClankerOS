/* panic.h - Kernel Panic Handler */
#ifndef PANIC_H
#define PANIC_H

#include "isr.h"

/*
 * KPanic - Kernel panic with detailed diagnostics
 *
 * Prints panic message to both VGA and serial console with source location,
 * then halts the system. This macro automatically captures file and line info.
 *
 * Usage:
 *   KPanic("Out of memory");
 *   KPanic("Invalid state: %d", someValue);
 */
#define KPanic(...) \
    KPanicImpl(__FILE__, __LINE__, __VA_ARGS__)

/*
 * KPanicRegs - Kernel panic with register dump
 *
 * Like KPanic but also dumps CPU register state.
 *
 * Usage:
 *   KPanicRegs(regs, "Page fault at %p", faultAddr);
 */
#define KPanicRegs(regs, ...) \
    KPanicWithRegsImpl(__FILE__, __LINE__, regs, __VA_ARGS__)

/*
 * KAssert - Kernel assertion
 *
 * Checks a condition and panics if it fails.
 *
 * Usage:
 *   KAssert(ptr != NULL, "Null pointer");
 *   KAssert(size > 0, "Invalid size: %u", size);
 */
#define KAssert(condition, ...) \
    do { \
        if (!(condition)) { \
            KPanicImpl(__FILE__, __LINE__, "Assertion failed: " #condition " - " __VA_ARGS__); \
        } \
    } while (0)

/*
 * KPanicImpl - Implementation function (do not call directly, use KPanic macro)
 *
 * @file: Source file name
 * @line: Source line number
 * @fmt: Printf-style format string
 * @...: Format arguments
 */
void KPanicImpl(const char* file, int line, const char* fmt, ...) __attribute__((noreturn));

/*
 * KPanicWithRegsImpl - Implementation with registers (do not call directly)
 *
 * @file: Source file name
 * @line: Source line number
 * @regs: CPU register state
 * @fmt: Printf-style format string
 * @...: Format arguments
 */
void KPanicWithRegsImpl(const char* file, int line, registers_t* regs,
                        const char* fmt, ...) __attribute__((noreturn));

#endif /* PANIC_H */
