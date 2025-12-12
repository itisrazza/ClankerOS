/* econ_writer.h - Early console writer interface */
#ifndef ECON_WRITER_H
#define ECON_WRITER_H

#include "clc/writer.h"

/*
 * EConGetWriter - Get ClcWriter interface for early console (COM1 serial)
 *
 * Returns a writer that outputs to the COM1 serial port for debugging.
 *
 * @return: Pointer to static ClcWriter for early console
 */
ClcWriter* EConGetWriter(void);

#endif /* ECON_WRITER_H */
