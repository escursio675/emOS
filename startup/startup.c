#include <stdint.h>

#define SRAM_END (0x20000000U + (96U * 1024U))  // F401RE: 96KB SRAM

/* Symbols provided by the linker script */
extern uint32_t _data;
extern uint32_t _edata;
extern uint32_t _data_loadaddr;
extern uint32_t _bss;
extern uint32_t _ebss;
extern int main(void);

void reset_handler(void);


// For future
// void default_handler(void);

/* Minimal vector table: stack pointer + reset handler.
 * Extend with real IRQ handlers in later phases (Phase 2). */
__attribute__((section(".vectors")))
void (* const vector_table[])(void) = {
    (void (*)(void))SRAM_END,  // initial stack pointer
    reset_handler,             // Reset handler
};

void reset_handler(void) {
    /* Copy .data from flash to RAM */
    uint32_t *src = &_data_loadaddr;
    uint32_t *dst = &_data;
    while (dst < &_edata) {
        *dst++ = *src++;
    }

   /* Zero .bss */
    uint32_t *bss_dst = &_bss;
    while (bss_dst < &_ebss) {
        *bss_dst++ = 0;
    }

    main();

    while (1) {
        // trap if main returns
    }
}