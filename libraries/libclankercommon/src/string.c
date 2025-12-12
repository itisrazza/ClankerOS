/* string.c - String utilities implementation */

#include "clc/string.h"
#include <stddef.h>
#include <stdbool.h>

/*
 * ClcStrLen - Get string length
 */
size_t ClcStrLen(const char* str)
{
    size_t len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}

/*
 * ClcStrCopy - Copy string
 */
void ClcStrCopy(char* dst, const char* src, size_t maxLen)
{
    size_t i = 0;
    while (i < maxLen - 1 && src[i]) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

/*
 * ClcStrEqual - Compare strings for equality
 */
bool ClcStrEqual(const char* a, const char* b)
{
    size_t i = 0;
    while (a[i] && b[i]) {
        if (a[i] != b[i]) {
            return false;
        }
        i++;
    }
    return a[i] == b[i];
}

/*
 * ClcStrStartsWith - Check if string starts with prefix
 */
bool ClcStrStartsWith(const char* str, const char* prefix)
{
    size_t i = 0;
    while (prefix[i]) {
        if (str[i] != prefix[i]) {
            return false;
        }
        i++;
    }
    return true;
}

/*
 * ClcStrCompare - Compare two strings lexicographically
 */
int ClcStrCompare(const char* a, const char* b)
{
    size_t i = 0;
    while (a[i] && b[i]) {
        if (a[i] != b[i]) {
            return (unsigned char)a[i] - (unsigned char)b[i];
        }
        i++;
    }
    return (unsigned char)a[i] - (unsigned char)b[i];
}
