/**
 * hexdump.h
 *
 * Low-level memory dump, independent of the UART. Output goes through a
 * callback so the shell (or a test) decides where the text ends up.
 *
 * Line format, same as `hexdump -C`:
 *   20000000  48 65 6c 6c 6f 20 77 6f  72 6c 64 0a 00 00 00 00  |Hello world.....|
 */
#ifndef HEXDUMP_H
#define HEXDUMP_H

#include <stddef.h>
#include <stdint.h>

#define HEXDUMP_BYTES_PER_LINE 16U
#define HEXDUMP_LINE_MAX       81U   /* longest line incl. "\r\n" and NUL */

typedef void (*hexdump_put_fn)(const char *s);

/* Format one line into `out` (HEXDUMP_LINE_MAX bytes). `addr` is the
 * address printed in the margin, `data` holds n (1..16) bytes.
 * Does no I/O and reads no memory itself. Returns the length written,
 * not counting the NUL. */
size_t hexdump_format_line(char *out, uint32_t addr, const uint8_t *data, size_t n);

/* 1 if [addr, addr+len) lies entirely in flash or SRAM, else 0.
 * Peripheral space is excluded on purpose: reserved gaps in it fault. */
int hexdump_range_valid(uint32_t addr, uint32_t len);

/* Dump `len` bytes starting at `addr`, one line per put() call.
 * Returns 0 on success (also for len == 0), -1 if the range is not valid
 * or put is NULL; nothing is read in that case. */
int hexdump(uint32_t addr, uint32_t len, hexdump_put_fn put);

#endif /* HEXDUMP_H */