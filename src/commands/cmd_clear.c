/**
 * cmd_clear.c
 *
 * `clear`: wipes the terminal screen (requirement F1).
 */

#include "commands/commands.h"
#include "shell/console.h"

cmd_result_t cmd_clear(void)
{
    console_clear();        /* ANSI: ESC[2J (erase screen) + ESC[H (cursor home) */
    return CMD_OK;
}