/**
 * console.c
 *
 * Terminal input/output for the shell.
 */

#include <stddef.h>
#include "shell/console.h"
#include "uart.h"

/* The one USART the console talks to. */
#define CONSOLE_USART  USART2

static key_filter_t s_filter;
static void (*s_idle_hook)(void) = NULL;

void console_init(void)
{
    key_filter_init(&s_filter);
    s_idle_hook = NULL;
}

void console_set_idle_hook(void (*hook)(void))
{
    s_idle_hook = hook;
}

/* ------------------------------------------------------------------ output */

void console_putc(char c)
{
    uart_putc(CONSOLE_USART, c);
}

void console_puts(const char *s)
{
    uart_puts(CONSOLE_USART, s);
}

void console_newline(void)
{
    console_puts("\r\n");
}

void console_put_uint(uint32_t v)
{
    char buf[11];                   /* 10 digits + NUL */
    uint32_t i = 10U;

    buf[i] = '\0';
    do {
        i--;
        buf[i] = (char)('0' + (v % 10U));
        v /= 10U;
    } while (v != 0U);

    console_puts(&buf[i]);
}

void console_put_hex32(uint32_t v)
{
    static const char digits[] = "0123456789abcdef";
    char buf[11];                   /* "0x" + 8 digits + NUL */
    int i;

    buf[0] = '0';
    buf[1] = 'x';
    for (i = 0; i < 8; i++) {
        buf[2 + i] = digits[(v >> (28 - 4 * i)) & 0xFU];
    }
    buf[10] = '\0';
    console_puts(buf);
}

void console_clear(void)
{
    /* ESC[2J = erase the whole screen, ESC[H = move the cursor to row 1 col 1.
     * These are standard ANSI codes interpreted by the terminal program. */
    console_puts("\033[2J\033[H");
}

/* ------------------------------------------------------------------- input */

int console_try_getkey(void)
{
    /* Feed every waiting byte through the filter until one produces a key.
     * Swallowed bytes (e.g. the 3 bytes of an arrow key) produce KEY_NONE,
     * so we simply keep going. */
    while (uart_data_available(CONSOLE_USART)) {
        uint8_t byte = (uint8_t)uart_getc(CONSOLE_USART);
        int key = key_filter_feed(&s_filter, byte);

        if (key != KEY_NONE) {
            return key;
        }
    }
    return KEY_NONE;
}

int console_wait_key(void)
{
    for (;;) {
        int key = console_try_getkey();

        if (key != KEY_NONE) {
            return key;
        }
        if (s_idle_hook != NULL) {
            s_idle_hook();
        }
    }
}

uint32_t console_readline(char *buf, uint32_t max)
{
    uint32_t count = 0U;

    if (buf == NULL || max == 0U) {
        return 0U;
    }

    for (;;) {
        int key = console_wait_key();

        if (key == KEY_ENTER) {
            break;
        }
        if (key == KEY_BACKSPACE) {
            if (count > 0U) {
                count--;
                console_puts("\b \b");   /* back, overwrite with space, back */
            }
            continue;
        }
        if (count < (max - 1U)) {       /* keep room for the '\0' */
            buf[count++] = (char)key;
            console_putc((char)key);    /* echo: the terminal does not */
        }
        /* else: line is full, ignore the key */
    }

    buf[count] = '\0';
    return count;
}