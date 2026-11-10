/**
 * console.h
 *
 * Everything the shell does with the terminal: printing text and reading
 * keys. Built on the USART2 driver (the ST-LINK virtual COM port).
 *
 * Output : console_puts / putc / put_uint / put_hex32 / newline / clear
 * Input  : console_try_getkey / wait_key / readline (filtered, see keyfilter.h)
 * Hook   : console_set_idle_hook() - code that runs while we wait for a key
 */

#ifndef SHELL_CONSOLE_H
#define SHELL_CONSOLE_H

#include <stdint.h>
#include "shell/keyfilter.h"     /* KEY_NONE, KEY_ENTER, KEY_BACKSPACE */

/* Resets the input filter and removes any idle hook.
 * Call once, AFTER uart_init(). */
void console_init(void);

/* Registers a function that is called repeatedly while the console waits for
 * a key. Pass NULL to remove it. The hook MUST be short and must not print:
 * the UART receiver holds only one byte, so a slow hook can lose keystrokes. */
void console_set_idle_hook(void (*hook)(void));

/* ---- output ---- */
void console_putc(char c);
void console_puts(const char *s);
void console_put_uint(uint32_t v);      /* decimal, no padding        */
void console_put_hex32(uint32_t v);     /* 0x%08x, lower case         */
void console_newline(void);             /* "\r\n"                     */
void console_clear(void);               /* ANSI: clear screen, home   */

/* ---- input ---- */

/* Non-blocking. Returns KEY_NONE if nothing usable has arrived, otherwise
 * KEY_ENTER, KEY_BACKSPACE, or the ASCII code of a letter/digit/space. */
int console_try_getkey(void);

/* Blocks until a usable key arrives; calls the idle hook while waiting. */
int console_wait_key(void);

/* Reads one line into buf (at most max-1 characters + terminating '\0').
 * Echoes typed characters, handles Backspace, stops at Enter.
 * Does NOT print the final newline: the caller does that.
 * When the line is full, further characters are ignored until Enter.
 * Returns the number of characters stored. */
uint32_t console_readline(char *buf, uint32_t max);

#endif /* SHELL_CONSOLE_H */