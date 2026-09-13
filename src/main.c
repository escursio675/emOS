#include <stdint.h>
#include "systick.h"
#include "gpio.h"

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
        .otype = GPIO_OTYPE_PUSH_PULL, /* ignored for INPUT mode */
        .speed = GPIO_SPEED_LOW,       /* ignored for INPUT mode */
        .pull  = GPIO_PULL_NONE,       /* B1 has an external pull-up on the Nucleo board itself */
        .af    = 0
    };

    /* LD2 on the Nucleo-F401RE is wired to PA5 */
    gpio_init(GPIOA, 5, &led_config);

    /* B1 user button is wired to PC13 (idle HIGH via external pull-up, LOW when pressed) */
    gpio_init(GPIOC, 13, &button_config);

    systick_init(16000 - 1);  /* 1ms tick @ 16MHz HSI */

    while (1) {
        if (gpio_read(GPIOC, 13) == GPIO_PIN_LOW) {
            /* Button pressed (pulled to GND): solid ON */
            gpio_write(GPIOA, 5, GPIO_PIN_HIGH);
        } else {
            /* Button released (pulled HIGH): blink as before */
            gpio_toggle(GPIOA, 5);
            delay_ms(500);
        }
    }

    return 0;
}