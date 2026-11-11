/**
 * commands.h
 *
 * Prototypes of every shell command handler. Each handler prints its own
 * output over the console and returns CMD_OK or CMD_FAIL.
 *
 * cmd_table.c includes this header because it must know every handler.
 * main.c also includes it, but only to call cmd_greet() once at boot.
 */

#ifndef COMMANDS_COMMANDS_H
#define COMMANDS_COMMANDS_H

#include "shell/shell.h"       /* cmd_result_t */

/* cmd_help.c */
cmd_result_t cmd_help(void);

/* cmd_clear.c */
cmd_result_t cmd_clear(void);

/* cmd_greet.c  (also called once from main.c at boot) */
cmd_result_t cmd_greet(void);

/* cmd_uptime.c */
cmd_result_t cmd_uptime(void);

/* cmd_hexdump.c */
cmd_result_t cmd_hexdump(void);

/* cmd_led.c  (the `led` menu: on, off, toggle, blinky) */
cmd_result_t cmd_led(void);

/* cmd_system.c */
cmd_result_t cmd_button(void);
cmd_result_t cmd_fault(void);

/* cmd_irq.c  (the `irq` menu: interrupt demo) */
cmd_result_t cmd_irq(void);

/* cmd_adc.c */
cmd_result_t cmd_adc(void);
cmd_result_t cmd_vref(void);

/* cmd_mem.c */
cmd_result_t cmd_mem(void);
cmd_result_t cmd_memtest(void);

/* cmd_selftest.c  (the `selftest` menu: GPIO, UART) */
cmd_result_t cmd_selftest(void);

/* cmd_version.c */
cmd_result_t cmd_version(void);

/* cmd_reboot.c */
cmd_result_t cmd_reboot(void);

#endif /* COMMANDS_COMMANDS_H */