/* vid_writer.c - VGA console writer implementation */

#include <stddef.h>
#include "clc/writer.h"

/* Forward declaration of VidPutChar from main.c */
extern void VidPutChar(char c);

/*
 * vidWriterPutChar - VGA writer putchar implementation
 */
static void vidWriterPutChar(void* data, char c)
{
    (void)data; // Unused for VGA writer
    VidPutChar(c);
}

/* VGA writer vtable */
static const ClcWriterVTable vidWriterVTable = {
    .putchar = vidWriterPutChar
};

/* Global VGA writer instance */
static ClcWriter vidWriterInstance = {
    .data = NULL,
    .vtable = &vidWriterVTable
};

/*
 * VidGetWriter - Get the VGA console writer
 */
ClcWriter* VidGetWriter(void)
{
    return &vidWriterInstance;
}
