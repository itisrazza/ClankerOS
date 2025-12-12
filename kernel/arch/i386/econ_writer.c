/* econ_writer.c - Early console writer implementation */

#include <stddef.h>
#include "econ_writer.h"
#include "early_console.h"
#include "clc/writer.h"

/* Writer vtable implementation */
static void econWriterPutChar(void* data, char c)
{
    (void)data;  // Unused
    EConPutChar(c);
}

static const ClcWriterVTable econVTable = {
    .putchar = econWriterPutChar
};

static ClcWriter econWriter = {
    .data = NULL,
    .vtable = &econVTable
};

/*
 * EConGetWriter - Get ClcWriter interface for early console
 */
ClcWriter* EConGetWriter(void)
{
    return &econWriter;
}
