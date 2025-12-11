/* writers.c - Common writer implementations for ClankerOS */

#include "clc/writers.h"
#include "clc/writer.h"

/*
 * bufferWriterPutChar - Buffer writer putchar implementation
 */
static void bufferWriterPutChar(void* data, char c)
{
    ClcBufferWriterContext* context = (ClcBufferWriterContext*)data;
    context->buffer[context->position++] = c;
}

/* Buffer writer vtable */
static const ClcWriterVTable bufferWriterVTable = {
    .putchar = bufferWriterPutChar
};

/*
 * ClcBufferWriterInit - Initialize a buffer writer
 */
ClcWriter ClcBufferWriterInit(char* buffer)
{
    static ClcBufferWriterContext context;
    context.buffer = buffer;
    context.position = 0;

    ClcWriter writer = {
        .data = &context,
        .vtable = &bufferWriterVTable
    };

    return writer;
}

/*
 * ClcBufferWriterGetPosition - Get current write position
 */
int ClcBufferWriterGetPosition(ClcWriter* writer)
{
    if (writer && writer->data) {
        ClcBufferWriterContext* context = (ClcBufferWriterContext*)writer->data;
        return context->position;
    }
    return 0;
}
