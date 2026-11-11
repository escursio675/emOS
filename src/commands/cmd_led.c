/**
 * cmd_led.c
 *
 * TEMPORARY LED commands (`led on`, `led off`, `led toggle`).
 * Phase 5 replaces these three with the single `led` menu.
 */

#include "commands/commands.h"
#include "shell/console.h"
#include "gpio.h"

/* LD2 on the Nucleo-F401RE is wired to PA5. */
#define LED_PORT  GPIOA
#define LED_PIN   5U

cmd_result_t cmd_led_on(void)
{
    gpio_write(LED_PORT, LED_PIN, GPIO_PIN_HIGH);
    console_puts("LED turned ON\r\n");
    return CMD_OK;
}

cmd_result_t cmd_led_off(void)
{
    gpio_write(LED_PORT, LED_PIN, GPIO_PIN_LOW);
    console_puts("LED turned OFF\r\n");
    return CMD_OK;
}

cmd_result_t cmd_led_toggle(void)
{
    gpio_toggle(LED_PORT, LED_PIN);
    console_puts("LED toggled\r\n");
    return CMD_OK;
}