/**
 * cmd_help.c
 *
 * `help`: lists every command in the table with its one-line summary.
 * It reads shell_commands[], so it can never be out of date.
 */

#include <stddef.h>
#include <stdint.h>
#include "commands/commands.h"
#include "shell/console.h"
#include "shell/shell.h"

/* Column where the summary starts (measured from the start of the name).
 * The longest name today is "selftest" (8). Names that are longer than
 * this still work: they just get one space before the summary. */
#define HELP_NAME_WIDTH  12U

cmd_result_t cmd_help(void)
{
    size_t i;

    console_puts("Available commands:\r\n");

    for (i = 0U; i < shell_command_count; i++) {
        const char *p = shell_commands[i].name;
        uint32_t    n = 0U;

        console_puts("  ");
        while (*p != '\0') {
            console_putc(*p);
            p++;
            n++;
        }
        do {                              /* pad to the summary column, at least one space */
            console_putc(' ');
            n++;
        } while (n < HELP_NAME_WIDTH);

        console_puts(shell_commands[i].summary);
        console_newline();
    }
    return CMD_OK;
}