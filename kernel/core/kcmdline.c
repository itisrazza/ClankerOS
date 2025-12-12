/* kcmdline.c - Kernel Command Line Parser Implementation */

#include "kcmdline.h"
#include "multiboot.h"
#include "clc/string.h"
#include <stddef.h>
#include <stdbool.h>

/* Maximum command line length */
#define CMDLINE_MAX_LEN 256

/* Parsed command line */
static char cmdLine[CMDLINE_MAX_LEN];
static bool cmdLineValid = false;

/*
 * KCmdLineInitialize - Parse kernel command line from multiboot
 */
void KCmdLineInitialize(multiboot_info_t* mbootInfo)
{
    // Check if command line is available
    if (!(mbootInfo->flags & (1 << 2))) {
        cmdLineValid = false;
        return;
    }

    // Copy command line to local buffer
    const char* bootCmdLine = (const char*)mbootInfo->cmdline;
    ClcStrCopy(cmdLine, bootCmdLine, CMDLINE_MAX_LEN);
    cmdLineValid = true;
}

/*
 * KCmdLineHasFlag - Check if a flag is present
 */
bool KCmdLineHasFlag(const char* flag)
{
    if (!cmdLineValid) {
        return false;
    }

    // Iterate through command line tokens
    const char* ptr = cmdLine;
    size_t flagLen = ClcStrLen(flag);

    while (*ptr) {
        // Skip whitespace
        while (*ptr == ' ' || *ptr == '\t') {
            ptr++;
        }

        if (*ptr == '\0') {
            break;
        }

        // Find end of token
        const char* tokenStart = ptr;
        while (*ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '=') {
            ptr++;
        }

        // Calculate token length
        size_t tokenLen = ptr - tokenStart;

        // Check if token matches flag
        if (tokenLen == flagLen) {
            bool match = true;
            for (size_t i = 0; i < flagLen; i++) {
                if (tokenStart[i] != flag[i]) {
                    match = false;
                    break;
                }
            }
            if (match) {
                return true;
            }
        }

        // Skip to next token
        while (*ptr && *ptr != ' ' && *ptr != '\t') {
            ptr++;
        }
    }

    return false;
}

/*
 * KCmdLineGetValue - Get value for a key=value argument
 */
const char* KCmdLineGetValue(const char* key)
{
    if (!cmdLineValid) {
        return NULL;
    }

    static char valueBuffer[CMDLINE_MAX_LEN];
    size_t keyLen = ClcStrLen(key);

    // Iterate through command line tokens
    const char* ptr = cmdLine;
    while (*ptr) {
        // Skip whitespace
        while (*ptr == ' ' || *ptr == '\t') {
            ptr++;
        }

        if (*ptr == '\0') {
            break;
        }

        // Check if this token starts with "key="
        if (ClcStrStartsWith(ptr, key) && ptr[keyLen] == '=') {
            // Found the key, extract value
            const char* valueStart = ptr + keyLen + 1;
            const char* valueEnd = valueStart;

            // Find end of value
            while (*valueEnd && *valueEnd != ' ' && *valueEnd != '\t') {
                valueEnd++;
            }

            // Copy value to buffer
            size_t valueLen = valueEnd - valueStart;
            if (valueLen >= CMDLINE_MAX_LEN) {
                valueLen = CMDLINE_MAX_LEN - 1;
            }

            for (size_t i = 0; i < valueLen; i++) {
                valueBuffer[i] = valueStart[i];
            }
            valueBuffer[valueLen] = '\0';

            return valueBuffer;
        }

        // Skip to next token
        while (*ptr && *ptr != ' ' && *ptr != '\t') {
            ptr++;
        }
    }

    return NULL;
}
