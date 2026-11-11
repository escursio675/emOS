/**
 * cmd_selftest.c
 *
 * `selftest`: a small menu over the two driver self-tests in selftest.c.
 * (It replaces the two-word commands `gpio test` and `uart test`, Phase 7.)
 *
 *     1) GPIO   2) UART   q) Quit
 *
 * selftest_gpio() and selftest_uart() print their own PASS/FAIL lines and
 * return nothing, so the actions always report CMD_OK. (If you later want the
 * status LED to flash red on a failed self-test, change those two functions
 * to return a result first.)
 */

#include <stddef.h>
#include "commands/commands.h"
#include "shell/console.h"
#include "shell/menu.h"
#include "selftest.h"

/* Throws away keys that are still waiting in the UART. selftest_uart() reads the
 * UART directly and asks the user to type fast, so a byte may be left over; the
 * menu must not read it as a menu key. */
static void st_drain_keys(void)
{
    while (console_try_getkey() != KEY_NONE) {
        /* drain */
    }
}

static cmd_result_t st_gpio(void)
{
    console_puts("GPIO self-test on PB0 (Arduino A3). Leave A3 unplugged.\r\n");
    selftest_gpio();
    st_drain_keys();
    return CMD_OK;
}

static cmd_result_t st_uart(void)
{
    console_puts("UART self-test: needs the keyboard, about 12 seconds.\r\n");
    selftest_uart();
    st_drain_keys();
    return CMD_OK;
}

cmd_result_t cmd_selftest(void)
{
    static const menu_item_t items[] = {
        { '1', "GPIO", st_gpio },
        { '2', "UART", st_uart }
    };

    return menu_run("Self-test menu", "selftest> ", items, sizeof(items) / sizeof(items[0]));
}