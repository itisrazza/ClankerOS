/* econ_writer.c - Early console writer implementation */

#include <stddef.h>
#include <stdbool.h>
#include "econ_writer.h"
#include "early_console.h"
#include "kcmdline.h"
#include "clc/writer.h"

/* Early console enabled flag */
static bool econEnabled = false;

/* Writer vtable implementation */
static void econWriterPutChar(void* data, char c)
{
    (void)data;  // Unused

    if (!econEnabled) {
        return;
    }

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
 * EConWriterEnable - Enable early console output
 *
 * Should be called after command line parsing if 'earlycon' flag is present.
 */
void EConWriterEnable(void)
{
    econEnabled = true;
}

/*
 * EConGetWriter - Get ClcWriter interface for early console
 */
ClcWriter* EConGetWriter(void)
{
    return &econWriter;
}
