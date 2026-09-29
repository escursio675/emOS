/**
 * uart.h
 *
 * Polling-mode UART driver for STM32F401RE.
 * Owner: P3 - Hardware Drivers
 *
 * Default target peripheral is USART2 on PA2 (TX) / PA3 (RX), since
 * that's the pin pair routed through the Nucleo board's ST-LINK VCP --
 * confirm this against whatever the shell subsystem expects.
 *
 * This is polling/blocking only, matching the proposal's core feature
 * set. Interrupt-driven RX with ring buffers is listed as a stretch
 * goal, so keep the API shape here in mind if that gets added later
 * (uart_data_available() is included now so callers don't need to
 * change when RX becomes non-blocking).
 */

#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stddef.h>
#include "stm32f401re.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize `usart` for 8N1 operation at `baudrate`, using APB clock
 * `pclk_hz` to compute the BRR divisor.
 *
 * Does NOT configure the associated GPIO pins into AF mode -- call
 * gpio_init() with GPIO_MODE_AF (and the correct AF number for the
 * chosen USART) before or after this, per the pinout the team settles
 * on.
 */
void uart_init(USART_TypeDef *usart, uint32_t baudrate, uint32_t pclk_hz);

/** Block until the TX register is free, then send one byte. */
void uart_putc(USART_TypeDef *usart, char c);

/** Block until a byte is received, then return it. */
char uart_getc(USART_TypeDef *usart);

/** Send a NUL-terminated string (blocking). */
void uart_puts(USART_TypeDef *usart, const char *s);

/** Send `len` raw bytes (blocking). */
void uart_write(USART_TypeDef *usart, const uint8_t *data, uint32_t len);

/** Non-blocking check: returns 1 if a received byte is waiting, else 0. */
int uart_data_available(USART_TypeDef *usart);

/**
 * Blocking line read for the interactive shell: reads bytes until '\r'
 * or '\n' (not included in output) or maxlen-1 bytes, then NUL-terminates.
 * Returns the number of characters read (excluding the NUL terminator).
 */
uint32_t uart_read_line(USART_TypeDef *usart, char *buf, uint32_t maxlen);

#ifdef __cplusplus
}
#endif

#endif /* UART_H */
