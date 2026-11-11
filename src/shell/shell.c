/**
 * shell.c
 *
 * The shell core: prompt loop and command dispatcher.
 *
 * Prints the prompt, reads a line, runs the matching command and reports an
 * unknown command without stopping. Leading and trailing blanks are ignored.
 * While a command runs the status LED is solid on; afterwards it flashes the
 * OK or ERROR pattern.
 */

#include <stddef.h>
#include "shell/shell.h"
#include "shell/console.h"
#include "shell/strutil.h"
#include "status_led.h"

void shell_print_commands(void)
{
    size_t i;

    for (i = 0U; i < shell_command_count; i++) {
        if (i > 0U) {
            console_puts(" | ");
        }
        console_puts(shell_commands[i].name);
    }
    console_newline();
}

cmd_result_t shell_execute(char *line)
{
    char  *cmd = str_trim(line);          /* strip leading and trailing blanks */
    size_t i;

    /* A blank line is not an error: just give the user a new prompt. */
    if (cmd == NULL || *cmd == '\0') {
        return CMD_OK;
    }

    for (i = 0U; i < shell_command_count; i++) {
        if (str_equals(cmd, shell_commands[i].name)) {
            cmd_result_t r;

            status_led_set(STATUS_BUSY);              /* solid on while it runs */
            r = shell_commands[i].run();
            status_led_signal((r == CMD_OK) ? STATUS_OK : STATUS_ERROR);
            return r;
        }
    }

    /* Not in the table: say so, point at help, keep running. */
    console_puts("Unknown command: ");
    console_puts(cmd);
    console_newline();
    if (str_has_space(cmd)) {
        console_puts("Hint: type the command exactly as shown by 'help', with one space between words.\r\n");
    }
    console_puts("Type 'help' to see the available commands.\r\n");
    status_led_signal(STATUS_ERROR);
    return CMD_FAIL;
}

void shell_run(void)
{
    char line[SHELL_LINE_MAX];

    for (;;) {
        console_puts("\r\n" SHELL_PROMPT);
        (void)console_readline(line, sizeof(line));
        console_newline();
        (void)shell_execute(line);
    }
}