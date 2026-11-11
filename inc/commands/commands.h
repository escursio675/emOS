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

/* cmd_greet.c  (also called once from main.c at boot: requirement N2) */
cmd_result_t cmd_greet(void);

/* cmd_uptime.c */
cmd_result_t cmd_uptime(void);

/* cmd_hexdump.c */
cmd_result_t cmd_hexdump(void);

/* cmd_led.c  (the `led` menu: on, off, toggle, blinky) */
cmd_result_t cmd_led(void);

/* cmd_system.c */
cmd_result_t cmd_button(void);
cmd_result_t cmd_stats(void);          /* TEMPORARY: folded into the `irq` menu in Phase 6 */
cmd_result_t cmd_fault(void);

/* cmd_adc.c */
cmd_result_t cmd_adc(void);
cmd_result_t cmd_adc_vref(void);       /* renamed `vref` in Phase 7 */

/* cmd_mem.c */
cmd_result_t cmd_mem(void);
cmd_result_t cmd_memtest(void);

/* cmd_selftest.c  (TEMPORARY: merged into the `selftest` menu in Phase 7) */
cmd_result_t cmd_gpio_test(void);
cmd_result_t cmd_uart_test(void);

#endif /* COMMANDS_COMMANDS_H */