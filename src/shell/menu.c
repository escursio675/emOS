/**
 * menu.c
 *
 * Single-key menu loop. Built on the console only (no drivers, no commands).
 */

#include <stddef.h>
#include "shell/menu.h"
#include "shell/console.h"

/* Prints the heading and the one-line list of keys. */
static void menu_print(const char *title, const menu_item_t *items, size_t count)
{
    size_t i;

    console_puts("\r\n=== ");
    console_puts(title);
    console_puts(" ===\r\n");

    for (i = 0U; i < count; i++) {
        console_putc(' ');
        console_putc(items[i].key);
        console_puts(") ");
        console_puts(items[i].label);
        console_puts("   ");
    }
    console_putc(' ');
    console_putc(MENU_QUIT_KEY);
    console_puts(") Quit\r\n");
}

cmd_result_t menu_run(const char *title, const char *prompt,
                      const menu_item_t *items, size_t count)
{
    menu_print(title, items, count);

    for (;;) {
        size_t i;
        int    found = 0;
        int    key;

        console_puts(prompt);
        key = console_wait_key();               /* ONE filtered key; runs the idle hook */

        if (key == KEY_ENTER) {                 /* just a fresh prompt */
            console_newline();
            continue;
        }
        if (key == KEY_BACKSPACE) {             /* nothing to erase in a menu */
            continue;
        }

        console_putc((char)key);                /* echo: the terminal does not */
        console_newline();

        if (key == MENU_QUIT_KEY || key == 'Q') {
            break;
        }

        for (i = 0U; i < count; i++) {
            if (key == (int)items[i].key) {
                (void)items[i].action();        /* the action prints its own result */
                found = 1;
                break;
            }
        }
        if (!found) {
            console_puts("Invalid choice. Press a menu key or q to quit.\r\n");
        }
    }

    /* Throw away keys typed ahead, so a stray key cannot start the next command. */
    while (console_try_getkey() != KEY_NONE) {
        /* drain */
    }
    return CMD_OK;
}