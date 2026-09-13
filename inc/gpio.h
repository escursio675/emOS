/**
 * gpio.h
 *
 * GPIO driver for STM32F401RE (Cortex-M4), bare-metal / no HAL.
 * Owner: P3 - Hardware Drivers
 *
 * Provides pin-level configuration and control on top of the raw
 * register definitions in stm32f401re.h.
 */

#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>
#include "stm32f401re.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------- */
/* Types                                                                 */
/* ------------------------------------------------------------------- */

typedef enum {
    GPIO_MODE_INPUT  = 0x0,
    GPIO_MODE_OUTPUT = 0x1,
    GPIO_MODE_AF     = 0x2,
    GPIO_MODE_ANALOG = 0x3
} gpio_mode_t;

typedef enum {
    GPIO_OTYPE_PUSH_PULL  = 0x0,
    GPIO_OTYPE_OPEN_DRAIN = 0x1
} gpio_otype_t;

typedef enum {
    GPIO_SPEED_LOW       = 0x0,
    GPIO_SPEED_MEDIUM    = 0x1,
    GPIO_SPEED_FAST      = 0x2,
    GPIO_SPEED_HIGH      = 0x3
} gpio_speed_t;

typedef enum {
    GPIO_PULL_NONE = 0x0,
    GPIO_PULL_UP   = 0x1,
    GPIO_PULL_DOWN = 0x2
} gpio_pull_t;

typedef enum {
    GPIO_PIN_LOW  = 0,
    GPIO_PIN_HIGH = 1
} gpio_state_t;

/* Simple config struct so gpio_init() doesn't need a huge argument list */
typedef struct {
    gpio_mode_t  mode;
    gpio_otype_t otype;   /* only relevant for GPIO_MODE_OUTPUT / AF */
    gpio_speed_t speed;
    gpio_pull_t  pull;
    uint8_t      af;      /* alternate function number 0-15, used if mode == GPIO_MODE_AF */
} gpio_config_t;

/* ------------------------------------------------------------------- */
/* API                                                                   */
/* ------------------------------------------------------------------- */

/**
 * Enable the peripheral clock for the given GPIO port.
 * Must be called (directly or via gpio_init) before touching a port's
 * registers.
 */
void gpio_clock_enable(GPIO_TypeDef *port);

/**
 * Configure a single pin according to `config`.
 * Handles clock enable, MODER, OTYPER, OSPEEDR, PUPDR, and AFR as needed.
 */
void gpio_init(GPIO_TypeDef *port, uint8_t pin, const gpio_config_t *config);

/** Set an already-AF-configured pin's alternate function number (0-15). */
void gpio_set_af(GPIO_TypeDef *port, uint8_t pin, uint8_t af);

/** Drive an output pin high or low. */
void gpio_write(GPIO_TypeDef *port, uint8_t pin, gpio_state_t state);

/** Flip an output pin's current state. */
void gpio_toggle(GPIO_TypeDef *port, uint8_t pin);

/** Read the current logic level of a pin (works for input or output pins). */
gpio_state_t gpio_read(GPIO_TypeDef *port, uint8_t pin);

#ifdef __cplusplus
}
#endif

#endif /* GPIO_H */
