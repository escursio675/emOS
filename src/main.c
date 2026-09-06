#include <stdint.h>

#define RCC_BASE     0x40023800U
#define RCC_AHB1ENR  (*(volatile uint32_t *)(RCC_BASE + 0x30))

#define GPIOA_BASE   0x40020000U
#define GPIOA_MODER  (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_ODR    (*(volatile uint32_t *)(GPIOA_BASE + 0x14))

int main(void) {
    /* Enable GPIOA clock (bit 0 of AHB1ENR) */
    RCC_AHB1ENR |= (1U << 0);

    /* Set PA5 as general purpose output (MODER5 = 01) */
    GPIOA_MODER &= ~(0x3U << (5 * 2));
    GPIOA_MODER |=  (0x1U << (5 * 2));

    while (1) {
        GPIOA_ODR ^= (1U << 5);  // toggle PA5

        for (volatile uint32_t i = 0; i < 500000; i++) {
            // crude busy-wait delay
        }
    }

    return 0;
}