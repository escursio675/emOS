/**
 * interrupts.h
 *
 * Core interrupt management, NVIC routing, and EXTI handlers for MiniOS.
 */

#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

/* Global Enable/Disable */
void irq_enable_global(void);
void irq_disable_global(void);

/* NVIC Configuration */
void nvic_enable_irq(uint8_t irqn);
void nvic_disable_irq(uint8_t irqn);
void nvic_set_priority(uint8_t irqn, uint8_t priority);

/* Peripheral Interrupt Setups */
void button_exti_init(void);

/* Exception & Interrupt Handlers */
void exti15_10_irqhandler(void);
void hardfault_handler(void);
void default_handler(void);

/* Global state tracked by the button interrupt */
extern volatile uint32_t button_press_count;

#endif /* INTERRUPTS_H */