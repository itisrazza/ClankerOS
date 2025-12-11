/* printf.c - Generic printf implementation for ClankerOS */

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include "clc/printf.h"
#include "clc/writer.h"
#include "clc/writers.h"

/* Forward declarations for internal helpers */
static int formatToWriter(ClcWriter* writer, const char* format, va_list args);
static int intToString(char* buffer, int32_t value, int base, int uppercase);
static int uintToString(char* buffer, uint32_t value, int base, int uppercase);

/*
 * ClcPrintfWriter - Formatted print to a writer
 */
int ClcPrintfWriter(ClcWriter* writer, const char* format, ...)
{
    va_list args;
    int result;

    va_start(args, format);
    result = ClcVPrintfWriter(writer, format, args);
    va_end(args);

    return result;
}

/*
 * ClcVPrintfWriter - Formatted print to a writer with va_list
 */
int ClcVPrintfWriter(ClcWriter* writer, const char* format, va_list args)
{
    return formatToWriter(writer, format, args);
}

/*
 * ClcSPrintf - Format string into buffer
 */
int ClcSPrintf(char* buffer, const char* format, ...)
{
    va_list args;
    int result;

    va_start(args, format);
    result = ClcVSPrintf(buffer, format, args);
    va_end(args);

    return result;
}

/*
 * ClcVSPrintf - Format string into buffer with va_list
 */
int ClcVSPrintf(char* buffer, const char* format, va_list args)
{
    // Create a buffer writer
    ClcWriter bufferWriter = ClcBufferWriterInit(buffer);

    // Use the common formatting logic
    int written = formatToWriter(&bufferWriter, format, args);

    // Null-terminate the buffer
    buffer[written] = '\0';

    return written;
}

/*
 * formatToWriter - Core formatting logic for writer output
 */
static int formatToWriter(ClcWriter* writer, const char* format, va_list args)
{
    int written = 0;
    const char* p = format;
    char tempBuffer[32];

    while (*p) {
        if (*p == '%') {
            p++;
            switch (*p) {
                case 's': {
                    const char* str = va_arg(args, const char*);
                    if (str == NULL) {
                        str = "(null)";
                    }
                    while (*str) {
                        ClcWriterPutChar(writer, *str++);
                        written++;
                    }
                    break;
                }

                case 'c': {
                    char c = (char)va_arg(args, int);
                    ClcWriterPutChar(writer, c);
                    written++;
                    break;
                }

                case 'd': {
                    int32_t value = va_arg(args, int32_t);
                    int len = intToString(tempBuffer, value, 10, 0);
                    for (int i = 0; i < len; i++) {
                        ClcWriterPutChar(writer, tempBuffer[i]);
                    }
                    written += len;
                    break;
                }

                case 'u': {
                    uint32_t value = va_arg(args, uint32_t);
                    int len = uintToString(tempBuffer, value, 10, 0);
                    for (int i = 0; i < len; i++) {
                        ClcWriterPutChar(writer, tempBuffer[i]);
                    }
                    written += len;
                    break;
                }

                case 'x': {
                    uint32_t value = va_arg(args, uint32_t);
                    int len = uintToString(tempBuffer, value, 16, 0);
                    for (int i = 0; i < len; i++) {
                        ClcWriterPutChar(writer, tempBuffer[i]);
                    }
                    written += len;
                    break;
                }

                case 'X': {
                    uint32_t value = va_arg(args, uint32_t);
                    int len = uintToString(tempBuffer, value, 16, 1);
                    for (int i = 0; i < len; i++) {
                        ClcWriterPutChar(writer, tempBuffer[i]);
                    }
                    written += len;
                    break;
                }

                case 'p': {
                    void* ptr = va_arg(args, void*);
                    ClcWriterPutChar(writer, '0');
                    ClcWriterPutChar(writer, 'x');
                    int len = uintToString(tempBuffer, (uint32_t)ptr, 16, 0);
                    for (int i = 0; i < len; i++) {
                        ClcWriterPutChar(writer, tempBuffer[i]);
                    }
                    written += len + 2;
                    break;
                }

                case '%': {
                    ClcWriterPutChar(writer, '%');
                    written++;
                    break;
                }

                default: {
                    // Unknown format specifier, just copy it
                    ClcWriterPutChar(writer, '%');
                    ClcWriterPutChar(writer, *p);
                    written += 2;
                    break;
                }
            }
            p++;
        } else {
            ClcWriterPutChar(writer, *p++);
            written++;
        }
    }

    return written;
}

/*
 * intToString - Convert signed integer to string
 */
static int intToString(char* buffer, int32_t value, int base, int uppercase)
{
    if (value < 0) {
        buffer[0] = '-';
        return 1 + uintToString(&buffer[1], (uint32_t)(-value), base, uppercase);
    }
    return uintToString(buffer, (uint32_t)value, base, uppercase);
}

/*
 * uintToString - Convert unsigned integer to string
 */
static int uintToString(char* buffer, uint32_t value, int base, int uppercase)
{
    const char* digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
    char temp[32];
    int i = 0;
    int written = 0;

    if (value == 0) {
        buffer[0] = '0';
        return 1;
    }

    // Convert to string in reverse
    while (value > 0) {
        temp[i++] = digits[value % base];
        value /= base;
    }

    // Reverse into output buffer
    while (i > 0) {
        buffer[written++] = temp[--i];
    }

    return written;
}
