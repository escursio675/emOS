#include <stdint.h>
#include "systick.h"

#define RCC_BASE     0x40023800U
#define RCC_AHB1ENR  (*(volatile uint32_t *)(RCC_BASE + 0x30))

#define GPIOA_BASE   0x40020000U
#define GPIOA_MODER  (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_ODR    (*(volatile uint32_t *)(GPIOA_BASE + 0x14))

int main(void) {
    RCC_AHB1ENR |= (1U << 0);
    GPIOA_MODER &= ~(0x3U << (5 * 2));
    GPIOA_MODER |=  (0x1U << (5 * 2));

    systick_init(16000 - 1);  // 1ms tick @ 16MHz HSI

    while (1) {
        GPIOA_ODR ^= (1U << 5);
        delay_ms(500);
    }
    return 0;
}