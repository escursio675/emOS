/**
 * uart.c
 *
 * Polling-mode UART driver implementation for STM32F401RE.
 * Owner: P3 - Hardware Drivers
 *
 * Status: uart_init(), TX path (putc/puts/write) implemented and
 * tested in Phase 3. RX path remains TODO for Phase 4.
 */

#include "uart.h"

void uart_init(USART_TypeDef *usart, uint32_t baudrate, uint32_t pclk_hz)
{
    if (usart == USART2) {
        RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    } else if (usart == USART1) {
        RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    } else if (usart == USART6) {
        RCC->APB2ENR |= RCC_APB2ENR_USART6EN;
    }

    usart->BRR = (pclk_hz + (baudrate / 2U)) / baudrate;

    usart->CR1 &= ~USART_CR1_M;
    usart->CR1 |= (USART_CR1_TE | USART_CR1_RE);
    usart->CR1 |= USART_CR1_UE;
}

void uart_putc(USART_TypeDef *usart, char c)
{
    /* Wait until the transmit data register is empty (previous byte,
     * if any, has moved into the shift register), then load the new
     * byte to send. */
    while (!(usart->SR & USART_SR_TXE)) { }
    usart->DR = (uint8_t)c;
}

char uart_getc(USART_TypeDef *usart)
{
    /* TODO (Phase 4):
     * while (!(usart->SR & USART_SR_RXNE)) { }
     * return (char)(usart->DR & 0xFF);
     */
    (void)usart;
    return 0;
}

void uart_puts(USART_TypeDef *usart, const char *s)
{
    while (*s != '\0') {
        uart_putc(usart, *s);
        s++;
    }
}

void uart_write(USART_TypeDef *usart, const uint8_t *data, uint32_t len)
{
    uint32_t i;
    for (i = 0; i < len; i++) {
        uart_putc(usart, (char)data[i]);
    }
}

int uart_data_available(USART_TypeDef *usart)
{
    /* TODO (Phase 4): return (usart->SR & USART_SR_RXNE) ? 1 : 0; */
    (void)usart;
    return 0;
}

uint32_t uart_read_line(USART_TypeDef *usart, char *buf, uint32_t maxlen)
{
    /* TODO (Phase 5):
     * Read chars via uart_getc() into buf until '\r'/'\n' or
     * (maxlen - 1) chars collected, then NUL-terminate.
     */
    (void)usart; (void)buf; (void)maxlen;
    return 0;
}