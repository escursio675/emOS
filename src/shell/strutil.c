/**
 * strutil.c
 *
 * Implementation of the shell string and number helpers.
 */

#include <stddef.h>
#include "shell/strutil.h"

static int is_space(char c)
{
    return (c == ' ' || c == '\t' || c == '\r' || c == '\n');
}

/* Value of one hex digit, or -1 if c is not a hex digit. */
static int hex_value(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

int str_equals(const char *a, const char *b)
{
    if (a == NULL || b == NULL) {
        return 0;
    }
    while (*a != '\0' && *b != '\0') {
        if (*a != *b) {
            return 0;
        }
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

char *str_trim(char *s)
{
    char *end;

    if (s == NULL) {
        return NULL;
    }

    /* 1. Skip leading white space. */
    while (is_space(*s)) {
        s++;
    }
    if (*s == '\0') {
        return s;                   /* nothing but blanks (or empty) */
    }

    /* 2. Find the last character, then walk back over trailing white space.
     *    *s is known to be non-blank here, so the walk-back always stops. */
    end = s;
    while (*end != '\0') {
        end++;
    }
    end--;                          /* now on the last real character */
    while (end > s && is_space(*end)) {
        end--;
    }
    end[1] = '\0';                  /* cut the line right after it */

    return s;
}

int str_has_space(const char *s)
{
    if (s == NULL) {
        return 0;
    }
    while (*s != '\0') {
        if (*s == ' ' || *s == '\t') {
            return 1;
        }
        s++;
    }
    return 0;
}

int parse_uint(const char *s, uint32_t *out)
{
    uint32_t v = 0U;

    if (s == NULL || out == NULL || *s == '\0') {
        return -1;
    }
    while (*s != '\0') {
        uint32_t d;

        if (*s < '0' || *s > '9') {
            return -1;
        }
        d = (uint32_t)(*s - '0');
        /* v * 10 + d must stay <= 0xFFFFFFFF, i.e. v <= (0xFFFFFFFF - d) / 10 */
        if (v > (0xFFFFFFFFU - d) / 10U) {
            return -1;
        }
        v = v * 10U + d;
        s++;
    }
    *out = v;
    return 0;
}

int parse_hex(const char *s, uint32_t *out)
{
    uint32_t v = 0U;

    if (s == NULL || out == NULL) {
        return -1;
    }
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        s += 2;                     /* optional prefix */
    }
    if (*s == '\0') {
        return -1;                  /* "" or just "0x" */
    }
    while (*s != '\0') {
        int d = hex_value(*s);

        if (d < 0) {
            return -1;
        }
        if (v > 0x0FFFFFFFU) {      /* another shift would drop the top digit */
            return -1;
        }
        v = (v << 4) | (uint32_t)d;
        s++;
    }
    *out = v;
    return 0;
}