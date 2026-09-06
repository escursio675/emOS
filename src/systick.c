#include <stdint.h>

#include "systick.h"

#define SYSTICK_BASE   0xE000E010U
#define SYSTICK_CTRL   (*(volatile uint32_t *)(SYSTICK_BASE + 0x00))
#define SYSTICK_LOAD   (*(volatile uint32_t *)(SYSTICK_BASE + 0x04))
#define SYSTICK_VAL    (*(volatile uint32_t *)(SYSTICK_BASE + 0x08))

#define SYSTICK_CTRL_ENABLE     (1U << 0)
#define SYSTICK_CTRL_TICKINT    (1U << 1)
#define SYSTICK_CTRL_CLKSOURCE  (1U << 2)

static volatile uint32_t system_ticks = 0;

void systick_init(uint32_t reload_value) {
    SYSTICK_LOAD = reload_value & 0x00FFFFFF;  // 24-bit max
    SYSTICK_VAL  = 0;
    SYSTICK_CTRL = SYSTICK_CTRL_CLKSOURCE | SYSTICK_CTRL_TICKINT | SYSTICK_CTRL_ENABLE;
}

void systick_handler(void) {
    system_ticks++;
}

uint32_t uptime_ms(void) {
    return system_ticks;
}

void delay_ms(uint32_t ms) {
    uint32_t start = system_ticks;
    while ((system_ticks - start) < ms) {
        // spin
    }
}