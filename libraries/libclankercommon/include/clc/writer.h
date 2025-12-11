/* clc/writer.h - Generic writer interface for ClankerOS */
#ifndef CLC_WRITER_H
#define CLC_WRITER_H

/*
 * ClcWriterVTable - Writer virtual function table
 *
 * This structure contains function pointers for writer operations,
 * similar to Rust trait vtables.
 */
typedef struct {
    void (*putchar)(void* data, char c);
} ClcWriterVTable;

/*
 * ClcWriter - Generic writer (wide pointer)
 *
 * This structure contains a pointer to the data object and a pointer
 * to its vtable, similar to Rust's wide pointers for trait objects.
 */
typedef struct {
    void* data;                      // Pointer to the actual writer object
    const ClcWriterVTable* vtable;   // Pointer to the vtable
} ClcWriter;

/*
 * ClcWriterPutChar - Write a single character using a writer
 */
static inline void ClcWriterPutChar(ClcWriter* writer, char c)
{
    if (writer && writer->vtable && writer->vtable->putchar) {
        writer->vtable->putchar(writer->data, c);
    }
}

#endif /* CLC_WRITER_H */
