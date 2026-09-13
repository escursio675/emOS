/**
 * stm32f401re.h
 *
 * Minimal, hand-written peripheral register map for the STM32F401RE
 * (ARM Cortex-M4). This is NOT the full ST CMSIS device header -- it only
 * defines what the GPIO / UART / ADC drivers in this project need.
 *
 * Reference: STM32F401xB/C/D/E Reference Manual (RM0368) and
 * STM32F401RE datasheet memory map.
 */

#ifndef STM32F401RE_H
#define STM32F401RE_H

#include <stdint.h>

/* ------------------------------------------------------------------- */
/* Base addresses                                                       */
/* ------------------------------------------------------------------- */

#define PERIPH_BASE         (0x40000000UL)
#define APB1PERIPH_BASE      PERIPH_BASE
#define APB2PERIPH_BASE     (PERIPH_BASE + 0x00010000UL)
#define AHB1PERIPH_BASE     (PERIPH_BASE + 0x00020000UL)

#define RCC_BASE            (AHB1PERIPH_BASE + 0x3800UL)

#define GPIOA_BASE           (AHB1PERIPH_BASE + 0x0000UL)
#define GPIOB_BASE           (AHB1PERIPH_BASE + 0x0400UL)
#define GPIOC_BASE           (AHB1PERIPH_BASE + 0x0800UL)
#define GPIOD_BASE           (AHB1PERIPH_BASE + 0x0C00UL)
#define GPIOE_BASE           (AHB1PERIPH_BASE + 0x1000UL)
#define GPIOH_BASE           (AHB1PERIPH_BASE + 0x1C00UL)

#define USART1_BASE          (APB2PERIPH_BASE + 0x1000UL)
#define USART2_BASE          (APB1PERIPH_BASE + 0x4400UL)
#define USART6_BASE          (APB2PERIPH_BASE + 0x1400UL)

#define ADC1_BASE             (APB2PERIPH_BASE + 0x2000UL)
#define ADC_COMMON_BASE       (APB2PERIPH_BASE + 0x2300UL)

/* ------------------------------------------------------------------- */
/* RCC                                                                   */
/* ------------------------------------------------------------------- */

typedef struct {
    volatile uint32_t CR;
    volatile uint32_t PLLCFGR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t AHB1RSTR;
    volatile uint32_t AHB2RSTR;
    uint32_t RESERVED0[2];
    volatile uint32_t APB1RSTR;
    volatile uint32_t APB2RSTR;
    uint32_t RESERVED1[2];
    volatile uint32_t AHB1ENR;
    volatile uint32_t AHB2ENR;
    uint32_t RESERVED2[2];
    volatile uint32_t APB1ENR;
    volatile uint32_t APB2ENR;
    /* remaining registers omitted -- add as needed */
} RCC_TypeDef;

#define RCC ((RCC_TypeDef *)RCC_BASE)

#define RCC_AHB1ENR_GPIOAEN   (1UL << 0)
#define RCC_AHB1ENR_GPIOBEN   (1UL << 1)
#define RCC_AHB1ENR_GPIOCEN   (1UL << 2)
#define RCC_AHB1ENR_GPIODEN   (1UL << 3)
#define RCC_AHB1ENR_GPIOEEN   (1UL << 4)
#define RCC_AHB1ENR_GPIOHEN   (1UL << 7)

#define RCC_APB1ENR_USART2EN  (1UL << 17)
#define RCC_APB2ENR_USART1EN  (1UL << 4)
#define RCC_APB2ENR_USART6EN  (1UL << 5)
#define RCC_APB2ENR_ADC1EN    (1UL << 8)

/* ------------------------------------------------------------------- */
/* GPIO                                                                  */
/* ------------------------------------------------------------------- */

typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFR[2]; /* AFR[0] = AFRL (pins 0-7), AFR[1] = AFRH (pins 8-15) */
} GPIO_TypeDef;

#define GPIOA ((GPIO_TypeDef *)GPIOA_BASE)
#define GPIOB ((GPIO_TypeDef *)GPIOB_BASE)
#define GPIOC ((GPIO_TypeDef *)GPIOC_BASE)
#define GPIOD ((GPIO_TypeDef *)GPIOD_BASE)
#define GPIOE ((GPIO_TypeDef *)GPIOE_BASE)
#define GPIOH ((GPIO_TypeDef *)GPIOH_BASE)

/* ------------------------------------------------------------------- */
/* USART                                                                 */
/* ------------------------------------------------------------------- */

typedef struct {
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t BRR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t GTPR;
} USART_TypeDef;

#define USART1 ((USART_TypeDef *)USART1_BASE)
#define USART2 ((USART_TypeDef *)USART2_BASE)
#define USART6 ((USART_TypeDef *)USART6_BASE)

#define USART_SR_TXE   (1UL << 7)
#define USART_SR_TC    (1UL << 6)
#define USART_SR_RXNE  (1UL << 5)

#define USART_CR1_UE   (1UL << 13)
#define USART_CR1_M    (1UL << 12)
#define USART_CR1_TE   (1UL << 3)
#define USART_CR1_RE   (1UL << 2)

/* ------------------------------------------------------------------- */
/* ADC                                                                   */
/* ------------------------------------------------------------------- */

typedef struct {
    volatile uint32_t SR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMPR1;
    volatile uint32_t SMPR2;
    volatile uint32_t JOFR[4];
    volatile uint32_t HTR;
    volatile uint32_t LTR;
    volatile uint32_t SQR1;
    volatile uint32_t SQR2;
    volatile uint32_t SQR3;
    volatile uint32_t JSQR;
    volatile uint32_t JDR[4];
    volatile uint32_t DR;
} ADC_TypeDef;

#define ADC1 ((ADC_TypeDef *)ADC1_BASE)

#define ADC_SR_EOC     (1UL << 1)
#define ADC_CR2_ADON   (1UL << 0)
#define ADC_CR2_SWSTART (1UL << 30)

#endif /* STM32F401RE_H */
