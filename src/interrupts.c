/**
 * interrupts.c
 *
 * Implementation of NVIC control, Exception Handlers, and Peripheral Interrupts.
 */

#include "interrupts.h"
#include "stm32f401re.h"
#include "uart.h"
#include "gpio.h"

volatile uint32_t button_press_count = 0;

/* ------------------------------------------------------------------- */
/* NVIC API                                                            */
/* ------------------------------------------------------------------- */

void irq_enable_global(void) {
    __asm volatile("cpsie i" : : : "memory");
}

void irq_disable_global(void) {
    __asm volatile("cpsid i" : : : "memory");
}

void nvic_enable_irq(uint8_t irqn) {
    NVIC->ISER[irqn >> 5] = (1UL << (irqn & 0x1F));
}

void nvic_disable_irq(uint8_t irqn) {
    NVIC->ICER[irqn >> 5] = (1UL << (irqn & 0x1F));
}

void nvic_set_priority(uint8_t irqn, uint8_t priority) {
    /* STM32F4 uses the top 4 bits of the 8-bit priority field */
    NVIC->IP[irqn] = (uint8_t)(priority << 4);
}

/* ------------------------------------------------------------------- */
/* EXTI: Blue Button (PC13)                                            */
/* ------------------------------------------------------------------- */

void button_exti_init(void) {
    /* 1. Enable SYSCFG clock */
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    /* 2. Route EXTI Line 13 to Port C (0x2) */
    /* EXTICR[3] covers EXTI lines 12-15. Line 13 is bits 4-7. */
    SYSCFG->EXTICR[3] &= ~(0xFU << 4);
    SYSCFG->EXTICR[3] |=  (0x2U << 4);

    /* 3. Configure EXTI Line 13 for Falling Edge Trigger */
    EXTI->FTSR |= (1UL << 13);

    /* 4. Unmask EXTI Line 13 */
    EXTI->IMR |= (1UL << 13);

    /* 5. Enable IRQ 40 (EXTI15_10) in NVIC */
    nvic_enable_irq(40);
}

/* Overrides the weak alias in startup.c */
void exti15_10_irqhandler(void) {
    /* Check if EXTI Line 13 caused the interrupt */
    if (EXTI->PR & (1UL << 13)) {
        
        /* Clear the pending flag by writing a 1! */
        EXTI->PR = (1UL << 13);

        /* Action: Toggle the LED and increment count */
        gpio_toggle(GPIOA, 5);
        button_press_count++;
    }
}

/* ------------------------------------------------------------------- */
/* Exception Handlers                                                  */
/* ------------------------------------------------------------------- */

/* Overrides the weak alias in startup.c */
void hardfault_handler(void) {
    irq_disable_global(); /* Stop all other interrupts */

    uart_puts(USART2, "\r\n===================================\r\n");
    uart_puts(USART2, " [KERNEL PANIC] HardFault Triggered\r\n");
    uart_puts(USART2, "===================================\r\n");
    uart_puts(USART2, "System Halted.\r\n");

    while (1) {
        // Halt
    }
}

/* Overrides the weak alias in startup.c */
void default_handler(void) {
    irq_disable_global();

    uart_puts(USART2, "\r\n[PANIC] Unhandled Interrupt!\r\n");

    while (1) {
        // Halt
    }
}