/**
 * cmd_selftest.c
 *
 * Thin command wrappers over the P3 driver self-tests in selftest.c.
 * TEMPORARY names (`gpio test`, `uart test`): Phase 7 merges them into the
 * single-word `selftest` menu.
 *
 * selftest_gpio() and selftest_uart() print their own PASS/FAIL lines and
 * return nothing, so these wrappers always report CMD_OK. (If you later want
 * the status LED to flash red on a failed self-test, change those two
 * functions to return a result first.)
 */

#include "commands/commands.h"
#include "selftest.h"

cmd_result_t cmd_gpio_test(void)
{
    selftest_gpio();
    return CMD_OK;
}

cmd_result_t cmd_uart_test(void)
{
    selftest_uart();
    return CMD_OK;
}