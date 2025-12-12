/* string.h - String utilities for ClankerOS */
#ifndef CLC_STRING_H
#define CLC_STRING_H

#include <stddef.h>
#include <stdbool.h>

/*
 * ClcStrLen - Get string length
 *
 * @param str: Null-terminated string
 * @return: Length of string (excluding null terminator)
 */
size_t ClcStrLen(const char* str);

/*
 * ClcStrCopy - Copy string
 *
 * Copies src to dst, up to maxLen characters (including null terminator).
 * Always null-terminates dst.
 *
 * @param dst: Destination buffer
 * @param src: Source string
 * @param maxLen: Maximum number of characters to copy (including null)
 */
void ClcStrCopy(char* dst, const char* src, size_t maxLen);

/*
 * ClcStrEqual - Compare strings for equality
 *
 * @param a: First string
 * @param b: Second string
 * @return: true if strings are equal, false otherwise
 */
bool ClcStrEqual(const char* a, const char* b);

/*
 * ClcStrStartsWith - Check if string starts with prefix
 *
 * @param str: String to check
 * @param prefix: Prefix to look for
 * @return: true if str starts with prefix, false otherwise
 */
bool ClcStrStartsWith(const char* str, const char* prefix);

/*
 * ClcStrCompare - Compare two strings lexicographically
 *
 * @param a: First string
 * @param b: Second string
 * @return: <0 if a<b, 0 if a==b, >0 if a>b
 */
int ClcStrCompare(const char* a, const char* b);

#endif /* CLC_STRING_H */
