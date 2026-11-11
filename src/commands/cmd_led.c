/**
 * cmd_led.c
 *
 * TEMPORARY LED commands: `led on`, `led off`, `led toggle`, `led auto`.
 * Phase 5 replaces all four with the single `led` menu.
 *
 * These no longer touch PA5 themselves: status_led owns the pin. `led on`,
 * `led off` and `led toggle` take MANUAL control (the status lights pause so
 * the LED stays as the user set it). `led auto` gives control back.
 */

#include "commands/commands.h"
#include "shell/console.h"
#include "status_led.h"

cmd_result_t cmd_led_on(void)
{
    status_led_acquire();
    status_led_manual_set(1);
    console_puts("LED turned ON\r\n");
    return CMD_OK;
}

cmd_result_t cmd_led_off(void)
{
    status_led_acquire();
    status_led_manual_set(0);
    console_puts("LED turned OFF\r\n");
    return CMD_OK;
}

cmd_result_t cmd_led_toggle(void)
{
    status_led_acquire();
    status_led_manual_toggle();
    console_puts("LED toggled\r\n");
    return CMD_OK;
}

cmd_result_t cmd_led_auto(void)
{
    status_led_release();
    console_puts("Status lights resumed\r\n");
    return CMD_OK;
}