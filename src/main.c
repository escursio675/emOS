#include <stdint.h>
#include "systick.h"
#include "gpio.h"
#include "uart.h"

int main(void)
{
    gpio_config_t led_config = {
        .mode  = GPIO_MODE_OUTPUT,
        .otype = GPIO_OTYPE_PUSH_PULL,
        .speed = GPIO_SPEED_LOW,
        .pull  = GPIO_PULL_NONE,
        .af    = 0
    };

    gpio_config_t button_config = {
        .mode  = GPIO_MODE_INPUT,
        .otype = GPIO_OTYPE_PUSH_PULL,
        .speed = GPIO_SPEED_LOW,
        .pull  = GPIO_PULL_NONE,
        .af    = 0
    };

    /* USART2 TX/RX pins: PA2 = TX, PA3 = RX, both use AF7 on the STM32F401RE */
    gpio_config_t usart2_pin_config = {
        .mode  = GPIO_MODE_AF,
        .otype = GPIO_OTYPE_PUSH_PULL,
        .speed = GPIO_SPEED_FAST,
        .pull  = GPIO_PULL_NONE,
        .af    = 7
    };

    gpio_init(GPIOA, 5, &led_config);
    gpio_init(GPIOC, 13, &button_config);
    gpio_init(GPIOA, 2, &usart2_pin_config);
    gpio_init(GPIOA, 3, &usart2_pin_config);

    /* HSI default clock: SYSCLK = 16MHz, APB1 prescaler = /1, so
     * USART2's peripheral clock (pclk1) is also 16MHz. */
    uart_init(USART2, 115200, 16000000);

    systick_init(16000 - 1);  /* 1ms tick @ 16MHz HSI */

    while (1) {
        if (gpio_read(GPIOC, 13) == GPIO_PIN_LOW) {
            gpio_write(GPIOA, 5, GPIO_PIN_HIGH);
        } else {
            gpio_toggle(GPIOA, 5);
            uart_puts(USART2, "Hello from MiniOS UART!\r\n");
            delay_ms(500);
        }
    }

    return 0;
}