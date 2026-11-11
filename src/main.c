#include <stdint.h>
#include "systick.h"
#include "gpio.h"
#include "uart.h"
#include "adc.h"
#include "memory.h"
#include "interrupts.h"
#include "status_led.h"
#include "shell/console.h"
#include "shell/shell.h"
#include "commands/commands.h"     /* only for cmd_greet() */

/* The button count the idle hook has already reacted to. */
static uint32_t s_last_button_count = 0U;

/* Runs repeatedly while the console waits for a key (console_wait_key).
 * RULES: keep it short, never print, never block: the UART receiver holds
 * only one byte, so a slow hook loses typed characters. */
static void idle_hook(void)
{
    uint32_t count = button_press_count;    /* one 32-bit read: atomic on Cortex-M */

    status_led_update();                    /* heartbeat / flash animation */

    if (count != s_last_button_count) {
        s_last_button_count = count;        /* remember it even if we show nothing */
        if (status_led_get() != STATUS_BUSY) {
            status_led_signal(STATUS_OK);   /* one short "button pressed" flash */
        }
    }
}

int main(void)
{
    /* LD2 (PA5) is configured by status_led_init() below: that module owns it. */

    /* B1 user button is wired to PC13 (idle HIGH via external pull-up, LOW when pressed) */
    gpio_init_input(GPIOC, 13);

    /* USART2 TX/RX pins: PA2 = TX, PA3 = RX, both use AF7 */
    gpio_init_af(GPIOA, 2, 7);
    gpio_init_af(GPIOA, 3, 7);

    /* PA0 (Arduino A0) as an analog input for ADC1 channel 0 */
    gpio_init_analog(GPIOA, 0);

    uart_init(USART2, 115200, 16000000);
    console_init();           /* after uart_init(): console sits on top of USART2 */
    systick_init(16000 - 1);  /* 1ms tick @ 16MHz HSI */
    kmalloc_init();
    adc_init();

    /* Initialize EXTI for Blue Button (NVIC Demo) */
    button_exti_init();
    irq_enable_global(); /* Ensure interrupts are unmasked */

    /* Status LED: after systick_init() because it reads uptime_ms().
     * The hook is registered AFTER console_init(), which clears any hook. */
    status_led_init();
    console_set_idle_hook(idle_hook);

    /* Boot greeting (N2): banner + "Type 'help' ..." hint. */
    (void)cmd_greet();

    shell_run();              /* never returns */

    return 0;
}