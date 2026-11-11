/**
 * strutil.h
 *
 * Small string and number helpers for the shell.
 * Written by hand so the project does not need <string.h> or <stdlib.h>.
 */

#ifndef SHELL_STRUTIL_H
#define SHELL_STRUTIL_H

#include <stdint.h>

/* Returns 1 if both strings are identical, 0 otherwise. NULL is never equal. */
int str_equals(const char *a, const char *b);

/* Removes leading and trailing white space (space, tab, CR, LF).
 * Works IN PLACE: the trailing white space is overwritten with '\0' and the
 * returned pointer points to the first non-blank character inside the same
 * buffer. Do not free() or modify the original pointer's meaning: keep using
 * the returned pointer. Returns NULL if s is NULL. */
char *str_trim(char *s);

/* Returns 1 if the string contains a space or tab, 0 otherwise. */
int str_has_space(const char *s);

/* Parses an unsigned decimal number ("0", "42", "4294967295").
 * Returns 0 and stores the value in *out on success.
 * Returns -1 (and leaves *out untouched) for an empty string, any non-digit
 * character, or a value that does not fit in 32 bits. */
int parse_uint(const char *s, uint32_t *out);

/* Parses an unsigned hexadecimal number with an optional "0x"/"0X" prefix
 * ("20000000", "0x0800ABCD", "ff"). Same return convention as parse_uint().
 * At most 8 hex digits fit in 32 bits; more is an error. */
int parse_hex(const char *s, uint32_t *out);

#endif /* SHELL_STRUTIL_H */