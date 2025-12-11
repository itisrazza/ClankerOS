/* vid_writer.h - VGA console writer interface */
#ifndef VID_WRITER_H
#define VID_WRITER_H

#include "clc/writer.h"

/*
 * VidGetWriter - Get the VGA console writer
 *
 * Returns a ClcWriter that outputs to the VGA text mode console.
 */
ClcWriter* VidGetWriter(void);

#endif /* VID_WRITER_H */
