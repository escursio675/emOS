/**
 * uart.c
 *
 * Polling-mode UART driver implementation for STM32F401RE.
 * Owner: P3 - Hardware Drivers
 *
 * Status: uart_init(), TX (putc/puts/write), RX (getc/data_available),
 * and uart_read_line() all implemented and tested.
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
    while (!(usart->SR & USART_SR_TXE)) { }
    usart->DR = (uint8_t)c;
}

char uart_getc(USART_TypeDef *usart)
{
    while (!(usart->SR & USART_SR_RXNE)) { }
    return (char)(usart->DR & 0xFF);
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
    return (usart->SR & USART_SR_RXNE) ? 1 : 0;
}

uint32_t uart_read_line(USART_TypeDef *usart, char *buf, uint32_t maxlen)
{
    uint32_t count = 0;
    char c;

    if (maxlen == 0U) {
        return 0U;
    }

    while (count < (maxlen - 1U)) {
        c = uart_getc(usart);

        /* Enter (either CR or LF) ends the line. */
        if (c == '\r' || c == '\n') {
            break;
        }

        /* Basic backspace handling for interactive shell UX: erase the
         * last buffered character both in memory and on the terminal
         * (backspace, overwrite with space, backspace again). */
        if ((c == '\b' || c == 0x7F) && count > 0U) {
            count--;
            uart_puts(usart, "\b \b");
            continue;
        }

        /* Ignore backspace when the buffer is already empty -- nothing
         * to erase, and ignore any other non-printable control byte
         * that isn't Enter or backspace. */
        if (c == '\b' || c == 0x7F) {
            continue;
        }

        buf[count] = c;
        count++;
        uart_putc(usart, c); /* local-style echo, since terminals like
                                 picocom have their own echo disabled */
    }

    buf[count] = '\0';
    return count;
}