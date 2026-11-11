/**
 * cmd_table.c
 *
 * THE table of shell commands. This is the only file that knows the full
 * list. To add a command: write the handler, declare it in commands.h, add
 * one row here, add one OBJS line to the Makefile, and add the name to
 * tests/host/test_table.c.
 *
 * Rules for `name` (requirement N8, checked by tests/host/test_table.c):
 *   - ONE word: lower case letters and digits only, starting with a letter
 *   - exactly what the user types (the match is case-sensitive)
 */

#include "shell/shell.h"
#include "commands/commands.h"

const shell_cmd_t shell_commands[] = {
    /* name          summary                                            handler          */
    { "help",       "Show this list of commands",                       cmd_help        },
    { "clear",      "Clear the terminal screen",                        cmd_clear       },
    { "greet",      "Show the welcome banner",                          cmd_greet       },
    { "uptime",     "Time since boot",                                  cmd_uptime      },
    { "hexdump",    "Dump memory (asks for address and length)",        cmd_hexdump     },
    { "led",        "LED menu: on, off, toggle, blinky",                cmd_led         },
    { "irq",        "Interrupt demo menu: status, trigger, wait, mask", cmd_irq         },
    { "button",     "Is the blue button pressed right now?",            cmd_button      },
    { "adc",        "Read A0: raw value and millivolts",                cmd_adc         },
    { "vref",       "Read the internal reference, estimate VDDA",       cmd_vref        },
    { "selftest",   "Self-test menu: GPIO, UART",                       cmd_selftest    },
    { "fault",      "Trigger a HardFault on purpose",                   cmd_fault       },
    { "mem",        "Heap statistics",                                  cmd_mem         },
    { "memtest",    "Scripted kmalloc/kfree demo",                      cmd_memtest     },
    { "version",    "Firmware name, build date and compiler",           cmd_version     },
    { "reboot",     "Restart the board (software reset)",               cmd_reboot      }
};

const size_t shell_command_count = sizeof(shell_commands) / sizeof(shell_commands[0]);