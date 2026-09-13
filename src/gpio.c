/**
 * gpio.c
 *
 * GPIO driver implementation for STM32F401RE.
 * Owner: P3 - Hardware Drivers
 */

#include "gpio.h"

void gpio_clock_enable(GPIO_TypeDef *port)
{
    uint32_t bit_index = ((uint32_t)port - GPIOA_BASE) / 0x400UL;
    RCC->AHB1ENR |= (1UL << bit_index);
}

void gpio_init(GPIO_TypeDef *port, uint8_t pin, const gpio_config_t *config)
{
    gpio_clock_enable(port);

    /* MODER: 2 bits per pin */
    port->MODER &= ~(0x3UL << (pin * 2));
    port->MODER |=  ((uint32_t)config->mode << (pin * 2));

    if (config->mode == GPIO_MODE_OUTPUT || config->mode == GPIO_MODE_AF) {
        /* OTYPER: 1 bit per pin */
        port->OTYPER &= ~(0x1UL << pin);
        port->OTYPER |=  ((uint32_t)config->otype << pin);

        /* OSPEEDR: 2 bits per pin */
        port->OSPEEDR &= ~(0x3UL << (pin * 2));
        port->OSPEEDR |=  ((uint32_t)config->speed << (pin * 2));
    }

    /* PUPDR: 2 bits per pin */
    port->PUPDR &= ~(0x3UL << (pin * 2));
    port->PUPDR |=  ((uint32_t)config->pull << (pin * 2));

    if (config->mode == GPIO_MODE_AF) {
        gpio_set_af(port, pin, config->af);
    }
}

void gpio_set_af(GPIO_TypeDef *port, uint8_t pin, uint8_t af)
{
    uint8_t idx    = pin / 8U;   /* 0 = AFRL (pins 0-7), 1 = AFRH (pins 8-15) */
    uint8_t offset = (pin % 8U) * 4U;

    port->AFR[idx] &= ~(0xFUL << offset);
    port->AFR[idx] |=  ((uint32_t)af << offset);
}

void gpio_write(GPIO_TypeDef *port, uint8_t pin, gpio_state_t state)
{
    if (state == GPIO_PIN_HIGH) {
        port->BSRR = (1UL << pin);
    } else {
        port->BSRR = (1UL << (pin + 16U));
    }
}

void gpio_toggle(GPIO_TypeDef *port, uint8_t pin)
{
    port->ODR ^= (1UL << pin);
}

gpio_state_t gpio_read(GPIO_TypeDef *port, uint8_t pin)
{
    return (port->IDR & (1UL << pin)) ? GPIO_PIN_HIGH : GPIO_PIN_LOW;
}