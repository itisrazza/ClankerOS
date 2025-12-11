/* early_console.h - Early boot console output (COM1 serial) */
#ifndef EARLY_CONSOLE_H
#define EARLY_CONSOLE_H

/* COM1 serial port for early debugging */
#define COM1_PORT 0x3F8

/* Public functions */
void EConInitialize(void);
void EConPutChar(char c);
void EConWriteString(const char* str);

#endif /* EARLY_CONSOLE_H */
