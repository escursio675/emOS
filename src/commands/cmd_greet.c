/**
 * cmd_greet.c
 *
 * `greet`: prints the welcome banner and a hint to use `help`.
 * main.c also calls it once at boot.
 */

#include "commands/commands.h"
#include "shell/console.h"

cmd_result_t cmd_greet(void)
{
    console_puts("\r\n");
    console_puts("+-----------------------------------------------------------------+\r\n");
    console_puts("|  emOS - bare-metal operating environment for STM32F401RE        |\r\n");
    console_puts("+-----------------------------------------------------------------+\r\n");
    console_puts("Welcome! Type 'help' to see the available commands.\r\n");
    return CMD_OK;
}