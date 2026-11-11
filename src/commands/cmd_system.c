/**
 * cmd_system.c
 *
 * Small system commands: `button`, `fault`.
 * (`stats` was folded into the `irq` menu in Phase 6.)
 */

#include "commands/commands.h"
#include "shell/console.h"
#include "gpio.h"

/* B1 user button is wired to PC13 (HIGH when released, LOW when pressed). */
#define BUTTON_PORT  GPIOC
#define BUTTON_PIN   13U

/* button: polls the button level right now (compare with the interrupt counter
 * in the `irq` menu, option 1). */
cmd_result_t cmd_button(void)
{
    if (gpio_read(BUTTON_PORT, BUTTON_PIN) == GPIO_PIN_LOW) {
        console_puts("Button is PRESSED\r\n");
    } else {
        console_puts("Button is NOT pressed\r\n");
    }
    return CMD_OK;
}

/* fault: deliberately executes an undefined instruction. The CPU raises a
 * UsageFault, which escalates to HardFault; hardfault_handler() prints the
 * panic message and halts. Press RESET to recover. */
cmd_result_t cmd_fault(void)
{
    console_puts("Simulating HardFault via undefined instruction.....\r\n");
    __asm volatile("udf #0");
    return CMD_FAIL;        /* never reached; keeps the function well-formed */
}