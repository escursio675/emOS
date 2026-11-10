/**
 * cmd_table.c
 *
 * THE table of shell commands. This is the only file that knows the full
 * list. To add a command: write the handler, declare it in commands.h, add
 * one row here, add one OBJS line to the Makefile.
 *
 * Rules for `name`:
 *   - lower case, letters and digits only
 *   - exactly what the user types (the match is case-sensitive)
 * Rows marked TEMPORARY keep their old two-word names until the phase named
 * in the comment turns them into single-word commands or menus.
 */

#include "shell/shell.h"
#include "commands/commands.h"

const shell_cmd_t shell_commands[] = {
    /* name          summary                                          handler          */
    { "help",       "Show this list of commands",                     cmd_help        },
    { "clear",      "Clear the terminal screen",                      cmd_clear       },
    { "greet",      "Show the welcome banner",                        cmd_greet       },
    { "uptime",     "Time since boot",                                cmd_uptime      },
    { "hexdump",    "Dump memory (asks for address and length)",      cmd_hexdump     },
    { "led on",     "Turn the LED on",                                cmd_led_on      },  /* TEMPORARY: `led` menu, Phase 5 */
    { "led off",    "Turn the LED off",                               cmd_led_off     },  /* TEMPORARY: `led` menu, Phase 5 */
    { "led toggle", "Flip the LED",                                   cmd_led_toggle  },  /* TEMPORARY: `led` menu, Phase 5 */
    { "button",     "Is the blue button pressed right now?",          cmd_button      },
    { "adc",        "Read A0: raw value and millivolts",              cmd_adc         },
    { "adc vref",   "Read the internal reference, estimate VDDA",     cmd_adc_vref    },  /* TEMPORARY: `vref`, Phase 7 */
    { "gpio test",  "GPIO self-test on PB0 (leave A3 unplugged)",     cmd_gpio_test   },  /* TEMPORARY: `selftest`, Phase 7 */
    { "uart test",  "UART self-test (needs the keyboard, ~12 s)",     cmd_uart_test   },  /* TEMPORARY: `selftest`, Phase 7 */
    { "stats",      "Button interrupt counter",                       cmd_stats       },  /* TEMPORARY: `irq` menu, Phase 6 */
    { "fault",      "Trigger a HardFault on purpose",                 cmd_fault       },
    { "mem",        "Heap statistics",                                cmd_mem         },
    { "memtest",    "Scripted kmalloc/kfree demo",                    cmd_memtest     }
};

const size_t shell_command_count = sizeof(shell_commands) / sizeof(shell_commands[0]);