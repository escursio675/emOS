/**
 * cmd_reboot.c
 *
 * `reboot`: software reset of the whole chip, same effect as the RESET button.
 *
 * Writes SCB->AIRCR with the key 0x05FA and SYSRESETREQ (bit 2), keeping the
 * PRIGROUP bits (10:8) as they are. Before that it waits until the last byte
 * of "Rebooting..." has left the UART (TC flag), with a loop limit so a broken
 * UART cannot hang the command.
 *
 * The three REBOOT_* macros wrap the hardware parts (TX-complete check,
 * barrier, halt loop) and can be overridden at compile time. On the board the
 * defaults below are used.
 */

#include <stdint.h>
#include "commands/commands.h"
#include "shell/console.h"
#include "interrupts.h"
#include "stm32f401re.h"

#define AIRCR_VECTKEY          (0x05FAUL << 16)
#define AIRCR_PRIGROUP_MASK    (0x7UL << 8)
#define AIRCR_SYSRESETREQ      (1UL << 2)

#define REBOOT_TX_WAIT_LOOPS   200000UL     /* upper limit for the TC wait */

#ifndef REBOOT_TX_DONE
#define REBOOT_TX_DONE()   ((USART2->SR & USART_SR_TC) != 0U)
#endif

#ifndef REBOOT_BARRIER
#define REBOOT_BARRIER()   __asm volatile("dsb" : : : "memory")
#endif

#ifndef REBOOT_HALT
#define REBOOT_HALT()      for (;;) { __asm volatile("nop"); }
#endif

cmd_result_t cmd_reboot(void)
{
    uint32_t n = REBOOT_TX_WAIT_LOOPS;

    console_puts("Rebooting...\r\n");

    /* Let the last character leave the UART before the chip resets. */
    while (!REBOOT_TX_DONE() && n > 0U) {
        n--;
    }

    irq_disable_global();               /* nothing may run between the write and the reset */
    SCB->AIRCR = AIRCR_VECTKEY | (SCB->AIRCR & AIRCR_PRIGROUP_MASK) | AIRCR_SYSRESETREQ;
    REBOOT_BARRIER();                   /* make sure the write has completed */
    REBOOT_HALT();                      /* the reset arrives within a few cycles */

    return CMD_FAIL;                    /* never reached on the board */
}