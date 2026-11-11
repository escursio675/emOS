/**
 * cmd_irq.c
 *
 * `irq`: the interrupt demo menu.
 *
 *     1) Status   2) Trigger   3) Wait for button   4) Mask/Unmask
 *     5) Reset counter   q) Quit
 *
 * It shows the whole path of the blue-button interrupt:
 *     PC13 -> EXTI line 13 -> NVIC IRQ 40 -> exti15_10_irqhandler()
 *          -> button_press_count
 *
 * This file is a register VIEWER: it reads EXTI and NVIC directly so the
 * user can see every gate and pending bit. It never writes the LED (the
 * status LED owns PA5) and it never changes how the ISR works.
 */

#include <stdint.h>
#include "commands/commands.h"
#include "shell/console.h"
#include "shell/menu.h"
#include "interrupts.h"
#include "stm32f401re.h"
#include "systick.h"

/* The blue button: EXTI line 13, which is served by IRQ 40 (EXTI15_10). */
#define IRQ_BUTTON_LINE      13U
#define IRQ_BUTTON_IRQN      40U
#define IRQ_LINE_MASK        (1UL << IRQ_BUTTON_LINE)
#define IRQ_NVIC_WORD        (IRQ_BUTTON_IRQN >> 5)           /* 40 / 32 = 1 */
#define IRQ_NVIC_MASK        (1UL << (IRQ_BUTTON_IRQN & 0x1FU))   /* bit 8   */

#define IRQ_WAIT_TIMEOUT_MS  10000U   /* "Wait for button" gives up after 10 s    */
#define IRQ_TRIGGER_WAIT_MS  10U      /* how long Trigger waits for the ISR       */
#define IRQ_SETTLE_MS        2U       /* pause so a just-unmasked ISR can finish  */

/* Sleep until the next interrupt. Can be overridden at compile time. */
#ifndef IRQ_WFI
#define IRQ_WFI()  __asm volatile("wfi" : : : "memory")
#endif

/* ---- small helpers ---- */

static int irq_is_enabled(void)
{
    return (NVIC->ISER[IRQ_NVIC_WORD] & IRQ_NVIC_MASK) != 0U;
}

static int irq_is_pending(void)
{
    return (NVIC->ISPR[IRQ_NVIC_WORD] & IRQ_NVIC_MASK) != 0U;
}

static void put_yes_no(const char *label, int yes)
{
    console_puts(label);
    console_puts(yes ? ": yes\r\n" : ": no\r\n");
}

/* Waits `ms` milliseconds on the SysTick clock (wrap-safe). */
static void irq_wait_ms(uint32_t ms)
{
    uint32_t t0 = uptime_ms();

    while ((uint32_t)(uptime_ms() - t0) < ms) {
        /* wait */
    }
}

/* Drops keys that are still waiting, so they cannot leak into the menu. */
static void irq_drain_keys(void)
{
    while (console_try_getkey() != KEY_NONE) {
        /* drain */
    }
}

/* ---- 1) Status ---- */

static cmd_result_t irq_status(void)
{
    console_puts("--- Interrupt status ---\r\n");

    console_puts("Button presses (ISR count): ");
    console_put_uint(button_press_count);
    console_newline();

    console_puts("Uptime (ms): ");
    console_put_uint(uptime_ms());              /* SysTick keeps interrupting */
    console_newline();

    put_yes_no("EXTI line 13 unmasked (IMR)", (EXTI->IMR & IRQ_LINE_MASK) != 0U);
    put_yes_no("EXTI line 13 pending (PR)",   (EXTI->PR  & IRQ_LINE_MASK) != 0U);
    put_yes_no("NVIC IRQ 40 enabled (ISER)",  irq_is_enabled());
    put_yes_no("NVIC IRQ 40 pending (ISPR)",  irq_is_pending());
    return CMD_OK;
}

/* ---- 2) Software trigger ---- */

static cmd_result_t irq_trigger(void)
{
    uint32_t before  = button_press_count;
    int      enabled = irq_is_enabled();
    uint32_t t0;

    console_puts("Writing EXTI->SWIER bit 13 (software trigger) ...\r\n");
    EXTI->SWIER |= (uint32_t)IRQ_LINE_MASK;

    if (enabled) {
        /* The ISR should run at once; allow a few ms, never block for long. */
        t0 = uptime_ms();
        while (button_press_count == before &&
               (uint32_t)(uptime_ms() - t0) < IRQ_TRIGGER_WAIT_MS) {
            /* wait */
        }
    }

    if (button_press_count != before) {
        console_puts("Interrupt fired. Count: ");
        console_put_uint(before);
        console_puts(" -> ");
        console_put_uint(button_press_count);
        console_newline();
        return CMD_OK;
    }

    if (!enabled) {
        console_puts("IRQ 40 is masked in the NVIC: the interrupt is pending but the ISR has not run. Count stays ");
        console_put_uint(button_press_count);
        console_puts(". Use menu key 4 to unmask.\r\n");
        return CMD_OK;
    }

    console_puts("No interrupt arrived (unexpected). Check menu key 1.\r\n");
    return CMD_FAIL;
}

/* ---- 3) Wait for the real button ---- */

static cmd_result_t irq_wait(void)
{
    uint32_t before = button_press_count;
    uint32_t start;

    if (!irq_is_enabled()) {
        console_puts("Note: IRQ 40 is masked (menu key 4), so a press will be remembered but not counted until you unmask.\r\n");
    }
    console_puts("Press the blue button (10 s timeout, any key cancels) ...\r\n");

    start = uptime_ms();
    while ((uint32_t)(uptime_ms() - start) < IRQ_WAIT_TIMEOUT_MS) {
        if (button_press_count != before) {
            console_puts("Button interrupt received. Count: ");
            console_put_uint(before);
            console_puts(" -> ");
            console_put_uint(button_press_count);
            console_newline();
            return CMD_OK;
        }
        if (console_try_getkey() != KEY_NONE) {
            irq_drain_keys();
            console_puts("Cancelled.\r\n");
            return CMD_OK;
        }
        IRQ_WFI();                              /* sleep until the next interrupt */
    }

    console_puts("Timeout: no button interrupt in 10 s.\r\n");
    return CMD_FAIL;
}

/* ---- 4) Mask / unmask ---- */

static cmd_result_t irq_mask(void)
{
    if (irq_is_enabled()) {
        nvic_disable_irq((uint8_t)IRQ_BUTTON_IRQN);
        console_puts("IRQ 40 is now MASKED (disabled in the NVIC). Presses are remembered but the ISR will not run.\r\n");
    } else {
        int was_pending = irq_is_pending();

        nvic_enable_irq((uint8_t)IRQ_BUTTON_IRQN);
        irq_wait_ms(IRQ_SETTLE_MS);             /* let a pending ISR run and finish */
        console_puts("IRQ 40 is now ENABLED. Count: ");
        console_put_uint(button_press_count);
        console_newline();
        if (was_pending) {
            console_puts("A press made while it was masked ran just now.\r\n");
        }
    }
    return CMD_OK;
}

/* ---- 5) Reset the counter ---- */

static cmd_result_t irq_reset(void)
{
    irq_disable_global();                       /* no ISR can run during the write */
    button_press_count = 0U;
    irq_enable_global();
    console_puts("Counter reset to 0.\r\n");
    return CMD_OK;
}

/* ---- the command ---- */

cmd_result_t cmd_irq(void)
{
    static const menu_item_t items[] = {
        { '1', "Status",          irq_status  },
        { '2', "Trigger",         irq_trigger },
        { '3', "Wait for button", irq_wait    },
        { '4', "Mask/Unmask",     irq_mask    },
        { '5', "Reset counter",   irq_reset   }
    };
    cmd_result_t result;

    result = menu_run("Interrupt menu", "irq> ", items, sizeof(items) / sizeof(items[0]));

    /* Never leave the button dead: if the user quit while masked, undo it. */
    if (!irq_is_enabled()) {
        nvic_enable_irq((uint8_t)IRQ_BUTTON_IRQN);
        console_puts("IRQ 40 was masked: it has been re-enabled.\r\n");
    }
    return result;
}