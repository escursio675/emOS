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
void nmi_handler(void);
void hardfault_handler(void);
void systick_handler(void);
void default_handler(void);


void default_handler(void) {
    while (1) {
        // trap — no UART yet to report faults
    }
}



/* Minimal vector table: stack pointer + reset handler.
 * Extended with IRQ handlers*/
__attribute__((section(".vectors")))
void (* const vector_table[])(void) = {
    (void (*)(void))SRAM_END,   // 0: initial stack pointer
    reset_handler,              // 1: Reset
    nmi_handler,                // 2: NMI
    hardfault_handler,          // 3: HardFault
    default_handler,            // 4: MemManage
    default_handler,            // 5: BusFault
    default_handler,            // 6: UsageFault
    0, 0, 0, 0,                 // 7-10: Reserved
    default_handler,            // 11: SVCall
    default_handler,            // 12: Debug Monitor
    0,                          // 13: Reserved
    default_handler,            // 14: PendSV
    systick_handler,            // 15: SysTick
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

void nmi_handler(void)       { default_handler(); }
void hardfault_handler(void) { default_handler(); }