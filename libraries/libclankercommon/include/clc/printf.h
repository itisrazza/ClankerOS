/* clc/printf.h - Generic printf implementation for ClankerOS */
#ifndef CLC_PRINTF_H
#define CLC_PRINTF_H

#include <stdarg.h>
#include "clc/writer.h"

/*
 * ClcPrintfWriter - Formatted print to a writer
 *
 * Supported format specifiers:
 *   %s - null-terminated string
 *   %c - single character
 *   %d - signed decimal integer
 *   %u - unsigned decimal integer
 *   %x - hexadecimal (lowercase)
 *   %X - hexadecimal (uppercase)
 *   %p - pointer (hexadecimal with 0x prefix)
 *   %% - literal '%' character
 *
 * Returns: number of characters written
 */
int ClcPrintfWriter(ClcWriter* writer, const char* format, ...);

/*
 * ClcVPrintfWriter - Formatted print to a writer with va_list
 *
 * Returns: number of characters written
 */
int ClcVPrintfWriter(ClcWriter* writer, const char* format, va_list args);

/*
 * ClcSPrintf - Format string into buffer
 *
 * Returns: number of characters written (excluding null terminator)
 */
int ClcSPrintf(char* buffer, const char* format, ...);

/*
 * ClcVSPrintf - Format string into buffer with va_list
 *
 * Returns: number of characters written (excluding null terminator)
 */
int ClcVSPrintf(char* buffer, const char* format, va_list args);

#endif /* CLC_PRINTF_H */
