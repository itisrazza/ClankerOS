/* clc/writers.h - Common writer implementations for ClankerOS */
#ifndef CLC_WRITERS_H
#define CLC_WRITERS_H

#include "clc/writer.h"

/*
 * Buffer writer - writes to a memory buffer
 */

typedef struct {
    char* buffer;
    int position;
} ClcBufferWriterContext;

/*
 * ClcBufferWriterInit - Initialize a buffer writer
 *
 * Creates a ClcWriter that writes to the specified buffer.
 * The caller is responsible for ensuring the buffer is large enough.
 *
 * Returns: An initialized ClcWriter
 */
ClcWriter ClcBufferWriterInit(char* buffer);

/*
 * ClcBufferWriterGetPosition - Get current write position
 *
 * Returns: Number of characters written so far
 */
int ClcBufferWriterGetPosition(ClcWriter* writer);

#endif /* CLC_WRITERS_H */
