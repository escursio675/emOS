/**
 * hexdump.c
 */
#include <stdint.h>
#include <stddef.h>
#include "../inc/hexdump.h"

/* STM32F401RE (RM0368): 512 KB flash, 96 KB SRAM */
#define HD_FLASH_START 0x08000000UL
#define HD_FLASH_END   0x0807FFFFUL
#define HD_SRAM_START  0x20000000UL
#define HD_SRAM_END    0x20017FFFUL

static const char hex_digits[] = "0123456789abcdef";

static char *put_hex(char *p, uint32_t value, uint32_t digits)
{
    while (digits > 0U) {
        digits--;
        *p++ = hex_digits[(value >> (digits * 4U)) & 0xFU];
    }
    return p;
}

size_t hexdump_format_line(char *out, uint32_t addr, const uint8_t *data, size_t n)
{
    char *p = out;
    size_t i;

    if (n > HEXDUMP_BYTES_PER_LINE) {
        n = HEXDUMP_BYTES_PER_LINE;
    }

    p = put_hex(p, addr, 8U);
    *p++ = ' ';
    *p++ = ' ';

    for (i = 0U; i < HEXDUMP_BYTES_PER_LINE; i++) {
        if (i < n) {
            p = put_hex(p, data[i], 2U);
            *p++ = ' ';
        } else {
            *p++ = ' ';   /* keep columns aligned on a short last line */
            *p++ = ' ';
            *p++ = ' ';
        }
        if (i == 7U || i == 15U) {
            *p++ = ' ';
        }
    }

    *p++ = '|';
    for (i = 0U; i < n; i++) {
        *p++ = (data[i] >= 0x20U && data[i] <= 0x7EU) ? (char)data[i] : '.';
    }
    *p++ = '|';
    *p++ = '\r';
    *p++ = '\n';
    *p   = '\0';

    return (size_t)(p - out);
}

static int in_region(uint32_t addr, uint32_t len, uint32_t start, uint32_t end)
{
    return (addr >= start) && (addr <= end) && ((len - 1U) <= (end - addr));
}

int hexdump_range_valid(uint32_t addr, uint32_t len)
{
    if (len == 0U) {
        return 0;
    }
    return in_region(addr, len, HD_FLASH_START, HD_FLASH_END) ||
           in_region(addr, len, HD_SRAM_START,  HD_SRAM_END);
}

int hexdump(uint32_t addr, uint32_t len, hexdump_put_fn put)
{
    uint8_t chunk[HEXDUMP_BYTES_PER_LINE];
    char    line[HEXDUMP_LINE_MAX];
    uint32_t done = 0U;

    if (len == 0U) {
        return 0;
    }
    if (put == NULL || !hexdump_range_valid(addr, len)) {
        return -1;
    }

    while (done < len) {
        uint32_t n = len - done;
        uint32_t i;

        if (n > HEXDUMP_BYTES_PER_LINE) {
            n = HEXDUMP_BYTES_PER_LINE;
        }
        for (i = 0U; i < n; i++) {
            chunk[i] = *(const volatile uint8_t *)(uintptr_t)(addr + done + i);
        }
        hexdump_format_line(line, addr + done, chunk, n);
        put(line);
        done += n;
    }

    return 0;
}