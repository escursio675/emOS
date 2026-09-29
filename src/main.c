#include <stdint.h>
#include "systick.h"
#include "gpio.h"
#include "uart.h"

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

    gpio_config_t led_config = {
        .mode  = GPIO_MODE_OUTPUT,
        .otype = GPIO_OTYPE_PUSH_PULL,
        .speed = GPIO_SPEED_LOW,
        .pull  = GPIO_PULL_NONE,
        .af    = 0
    };

    gpio_config_t button_config = {
        .mode  = GPIO_MODE_INPUT,
        .otype = GPIO_OTYPE_PUSH_PULL, /* ignored for INPUT mode */
        .speed = GPIO_SPEED_LOW,       /* ignored for INPUT mode */
        .pull  = GPIO_PULL_NONE,       /* B1 has an external pull-up on the Nucleo board itself */
        .af    = 0
    };

    gpio_config_t usart2_pin_config = {
        .mode  = GPIO_MODE_AF,
        .otype = GPIO_OTYPE_PUSH_PULL,
        .speed = GPIO_SPEED_FAST,
        .pull  = GPIO_PULL_NONE,
        .af    = 7
    };

    /* LD2 on the Nucleo-F401RE is wired to PA5 */
    gpio_init(GPIOA, 5, &led_config);

    /* B1 user button is wired to PC13 (idle HIGH via external pull-up, LOW when pressed) */
    gpio_init(GPIOC, 13, &button_config);

    /* USART2 TX/RX pins: PA2 = TX, PA3 = RX, both use AF7 */
    gpio_init(GPIOA, 2, &usart2_pin_config);
    gpio_init(GPIOA, 3, &usart2_pin_config);

    uart_init(USART2, 115200, 16000000);
    systick_init(16000 - 1);  /* 1ms tick @ 16MHz HSI */

    uart_puts(USART2, "\r\n--- MiniOS P3 Driver Integration Test ---\r\n");
    uart_puts(USART2, "Commands: led on | led off | led toggle | button\r\n");

    while (1) {
        uart_puts(USART2, "\r\n> ");
        uart_read_line(USART2, line_buf, sizeof(line_buf));
        uart_puts(USART2, "\r\n");

        if (str_equals(line_buf, "led on")) {
            gpio_write(GPIOA, 5, GPIO_PIN_HIGH);
            uart_puts(USART2, "LED turned ON\r\n");
        } else if (str_equals(line_buf, "led off")) {
            gpio_write(GPIOA, 5, GPIO_PIN_LOW);
            uart_puts(USART2, "LED turned OFF\r\n");
        } else if (str_equals(line_buf, "led toggle")) {
            gpio_toggle(GPIOA, 5);
            uart_puts(USART2, "LED toggled\r\n");
        } else if (str_equals(line_buf, "button")) {
            if (gpio_read(GPIOC, 13) == GPIO_PIN_LOW) {
                uart_puts(USART2, "Button is PRESSED\r\n");
            } else {
                uart_puts(USART2, "Button is NOT pressed\r\n");
            }
        } else if (line_buf[0] == '\0') {
            /* Empty line (just pressed Enter) -- do nothing, reprint prompt. */
        } else {
            uart_puts(USART2, "Unknown command. Try: led on / led off / led toggle / button\r\n");
        }
    }

    return 0;
}