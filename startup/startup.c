#include <stdint.h>

#define SRAM_END (0x20000000U + (96U * 1024U))  // F401RE: 96KB SRAM

/* Symbols provided by the linker script */
extern uint32_t _data;
extern uint32_t _edata;
extern uint32_t _data_loadaddr;
extern uint32_t _bss;
extern uint32_t _ebss;
extern int main(void);

/* Handler Declarations */

void reset_handler(void);
/* a weak default handler. any unhandled interrupt falls here. */
void default_handler(void) __attribute__((weak));
void default_handler(void) {
    while (1) {
        // trap — unhandled interrupt
    }
}
/* weak Aliases for Core Exceptions */
void nmi_handler(void) __attribute__((weak, alias("default_handler")));
void hardfault_handler(void) __attribute__((weak, alias("default_handler")));
extern void systick_handler(void); /* implemented in systick.c */
/* Weak Aliases for Peripheral Interrupts (IRQs) */
void exti15_10_irqhandler(void) __attribute__((weak, alias("default_handler")));



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

    /* External Interrupts (IRQs) start at index 16 */
    [16 ... 55] = default_handler,  // IRQ 0 to 39 default mapped
    exti15_10_irqhandler,           // 56: IRQ 40 (EXTI15_10)
    [57 ... 100] = default_handler  // IRQ 41 to 84 default mapped
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