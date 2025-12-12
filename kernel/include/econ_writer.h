/* econ_writer.h - Early console writer interface */
#ifndef ECON_WRITER_H
#define ECON_WRITER_H

#include "clc/writer.h"

/*
 * EConWriterEnable - Enable early console output
 *
 * Call this after parsing command line if 'earlycon' flag is present.
 * Until this is called, EConGetWriter() returns a no-op writer.
 */
void EConWriterEnable(void);

/*
 * EConGetWriter - Get ClcWriter interface for early console (COM1 serial)
 *
 * Returns a writer that outputs to the COM1 serial port for debugging.
 * Output is no-op unless EConWriterEnable() has been called.
 *
 * @return: Pointer to static ClcWriter for early console
 */
ClcWriter* EConGetWriter(void);

#endif /* ECON_WRITER_H */
