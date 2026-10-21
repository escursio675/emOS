#include <stdint.h>
#include "systick.h"
#include "gpio.h"
#include "uart.h"
#include "interrupts.h"

/* Small local string-equality helper -- avoids pulling in <string.h>
 * for a single comparison, matching this project's minimal-dependency
 * bare-metal style. */
static int str_equals(const char *a, const char *b)
{
    while (*a != '\0' && *b != '\0') {
        if (*a != *b) {
            return 0;
        }
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

int main(void)
{
    char line_buf[64];

    /* LD2 on the Nucleo-F401RE is wired to PA5 */
    gpio_init_output(GPIOA, 5);

    /* B1 user button is wired to PC13 (idle HIGH via external pull-up, LOW when pressed) */
    gpio_init_input(GPIOC, 13);

    /* USART2 TX/RX pins: PA2 = TX, PA3 = RX, both use AF7 */
    gpio_init_af(GPIOA, 2, 7);
    gpio_init_af(GPIOA, 3, 7);

    uart_init(USART2, 115200, 16000000);
    systick_init(16000 - 1);  /* 1ms tick @ 16MHz HSI */

    /* Initialize EXTI for Blue Button (NVIC Demo) */
    button_exti_init();
    irq_enable_global(); /* Ensure interrupts are unmasked */

    uart_puts(USART2, "\r\n--- emOS Core Interrupts Booted ---\r\n");
    uart_puts(USART2, "Commands: led on | led off | led toggle | stats | fault\r\n");
    uart_puts(USART2, "Press the Blue Button at any time to trigger an interrupt!\r\n");

    while (1) {
        uart_puts(USART2, "\r\n> ");
        uart_read_line(USART2, line_buf, sizeof(line_buf));
        uart_puts(USART2, "\r\n");

        if (str_equals(line_buf, "led on")) {
            gpio_write(GPIOA, 5, GPIO_PIN_HIGH);
            uart_puts(USART2, "LED turned ON\r\n");
        }

        else if (str_equals(line_buf, "led off")) {
            gpio_write(GPIOA, 5, GPIO_PIN_LOW);
            uart_puts(USART2, "LED turned OFF\r\n");
        }

        else if (str_equals(line_buf, "led toggle")) {
            gpio_toggle(GPIOA, 5);
            uart_puts(USART2, "LED toggled\r\n");
        }

        else if (str_equals(line_buf, "stats")) {

            /* basic integer printing to show the interrupt variable changing */

            uart_puts(USART2, "Button interrupt trigger count: ");
            char count_str[10];
            int i = 0;
            uint32_t temp = button_press_count;
            if (temp == 0) count_str[i++] = '0';
            while (temp > 0 && i < 9) { count_str[i++] = (temp % 10) + '0'; temp /= 10; }
            while (i > 0) uart_putc(USART2, count_str[--i]);
            uart_puts(USART2, "\r\n");
        }

        else if (str_equals(line_buf, "fault")) {
            uart_puts(USART2, "Simulating HardFault by dereferencing invalid address...\r\n");
            volatile uint32_t *bad_ptr = (volatile uint32_t *)0xFFFFFFF0;
            *bad_ptr = 0xDEADBEEF; /* This will instantly trigger a HardFault */
        }

        else if (line_buf[0] == '\0') {
            /* Empty line (just pressed Enter) -- do nothing, reprint prompt. */
        }

        else {
            uart_puts(USART2, "Unknown command. Try: led on / led off / led toggle / button\r\n");
        }
    }

    return 0;
}