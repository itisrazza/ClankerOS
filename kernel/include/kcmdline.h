/* kcmdline.h - Kernel Command Line Parser */
#ifndef KCMDLINE_H
#define KCMDLINE_H

#include <stdbool.h>
#include "multiboot.h"

/*
 * KCmdLineInitialize - Parse kernel command line from multiboot
 *
 * Extracts and parses the kernel command line passed by the bootloader.
 * Must be called early in KMain() before using KCmdLineHasFlag() or
 * KCmdLineGetValue().
 *
 * @param mbootInfo: Multiboot information structure
 */
void KCmdLineInitialize(multiboot_info_t* mbootInfo);

/*
 * KCmdLineHasFlag - Check if a flag is present in the command line
 *
 * Checks for the presence of a boolean flag (e.g., "earlycon", "boottest").
 *
 * @param flag: Flag name to check for
 * @return: true if flag is present, false otherwise
 */
bool KCmdLineHasFlag(const char* flag);

/*
 * KCmdLineGetValue - Get value for a key=value argument
 *
 * Retrieves the value associated with a key (e.g., "console=ttyS0").
 *
 * @param key: Key name to look for
 * @return: Pointer to value string, or NULL if key not found
 */
const char* KCmdLineGetValue(const char* key);

#endif /* KCMDLINE_H */
