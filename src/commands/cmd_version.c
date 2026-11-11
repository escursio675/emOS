/**
 * cmd_version.c
 *
 * `version`: firmware name, build date and time, compiler, target.
 *
 * __DATE__ and __TIME__ are set when THIS file is compiled. make only
 * recompiles it when it changes, so after editing other files run
 * `touch src/commands/cmd_version.c` (or `make clean`) if you want the time to
 * match the build.
 */

#include "commands/commands.h"
#include "shell/console.h"

#define FW_NAME  "emOS"

cmd_result_t cmd_version(void)
{
    console_puts(FW_NAME " shell firmware\r\n");
    console_puts("Built:    " __DATE__ " " __TIME__ "\r\n");
    console_puts("Compiler: " __VERSION__ "\r\n");
    console_puts("Target:   STM32F401RE (Cortex-M4, 16 MHz HSI)\r\n");
    return CMD_OK;
}