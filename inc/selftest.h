/**
 * selftest.h
 *
 * Shell self-tests for the P3 drivers: `gpio test` and `uart test`.
 * Owner: P3 - Hardware Drivers
 *
 * Both print their progress and results over USART2, so the UART and GPIO
 * drivers must already be initialised.
 */

#ifndef SELFTEST_H
#define SELFTEST_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Wire-free GPIO test on PB0 (Arduino A3). Checks port B's clock, pull-up and
 * pull-down, push-pull and open-drain outputs, and toggling, by reading the pin
 * back after each setting. Nothing may be plugged into A3. Leaves PB0 as a
 * floating input.
 */
void selftest_gpio(void);

/**
 * UART test with three stages: uart_write sends exactly the length asked for,
 * uart_data_available and uart_getc see a key press, and bytes that arrive
 * while nothing is reading are counted as overruns. Needs a person at the
 * keyboard and takes about 12 seconds.
 */
void selftest_uart(void);

#ifdef __cplusplus
}
#endif

#endif /* SELFTEST_H */