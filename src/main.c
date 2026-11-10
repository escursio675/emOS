#include <stdint.h>
#include <stddef.h>
#include "systick.h"
#include "gpio.h"
#include "uart.h"
#include "adc.h"
#include "selftest.h"
#include "../inc/memory.h"
#include "interrupts.h"
#include "shell/console.h"
#include "shell/strutil.h"


/* mem: allocator statistics */
static void cmd_mem(void)
{
    heap_stats_t s;

    kmalloc_stats(&s);

    console_puts("Heap:\r\n");
    console_puts("  Total:        "); console_put_uint((uint32_t)s.total_bytes);  console_puts("\r\n");
    console_puts("  Used:         "); console_put_uint((uint32_t)s.used_bytes);   console_puts("\r\n");
    console_puts("  Free:         "); console_put_uint((uint32_t)s.free_bytes);   console_puts("\r\n");
    console_puts("  Largest free: "); console_put_uint((uint32_t)s.largest_free); console_puts("\r\n");
    console_puts("  Blocks:       "); console_put_uint((uint32_t)s.block_count);  console_puts("\r\n");
}

/* memtest: exercises kmalloc/kfree on the board. Temporary test command. */
static void cmd_memtest(void)
{
    void *a = kmalloc(100);
    void *b = kmalloc(200);
    void *c;
    void *big;

    console_puts("a = kmalloc(100): "); console_put_hex32((uint32_t)(uintptr_t)a); console_puts("\r\n");
    console_puts("b = kmalloc(200): "); console_put_hex32((uint32_t)(uintptr_t)b); console_puts("\r\n");
    cmd_mem();

    kfree(a);
    c = kmalloc(50);
    console_puts("kfree(a); c = kmalloc(50): "); console_put_hex32((uint32_t)(uintptr_t)c);
    console_puts(c == a ? " (reused a's block)\r\n" : " (different block)\r\n");

    big = kmalloc(KHEAP_SIZE);
    console_puts("kmalloc(KHEAP_SIZE): ");
    console_puts(big == NULL ? "NULL (out of memory, as expected)\r\n" : "UNEXPECTED: succeeded\r\n");

    kfree(b);
    kfree(c);
    kfree(big);              /* NULL is ignored */
    console_puts("freed everything\r\n");
    cmd_mem();
}

/* adc: one conversion on A0 (PA0 = ADC1 channel 0), shown as raw count and millivolts */
static void cmd_adc(void)
{
    uint16_t raw = adc_read(ADC_CHANNEL_0);

    if (raw == ADC_READ_ERROR) {
        console_puts("ADC read failed: no end-of-conversion\r\n");
        return;
    }

    console_puts("ADC A0 (PA0): raw "); console_put_uint(raw);
    console_puts(" of 4095, ");         console_put_uint(adc_to_millivolts(raw));
    console_puts(" mV\r\n");
}

/* adc vref: reads the chip's internal 1.21 V reference and estimates VDDA from it.
 * Needs no wiring, so it checks the whole conversion path on its own. */
static void cmd_adc_vref(void)
{
    uint16_t raw = adc_read(ADC_CHANNEL_VREFINT);

    if (raw == ADC_READ_ERROR) {
        console_puts("ADC read failed: no end-of-conversion\r\n");
        return;
    }

    console_puts("VREFINT raw "); console_put_uint(raw);
    console_puts(" of 4095, estimated VDDA "); console_put_uint(adc_estimate_vdda_mv(raw));
    console_puts(" mV\r\n");
    console_puts("(expected: raw about 1450-1550, VDDA about 3200-3400 mV)\r\n");
}

int main(void)
{
    char line_buf[64];

    /* LD2 on the Nucleo-F401RE is wired to PA5 */
    gpio_init_output(GPIOA, 5);

    /* B1 user button is wired to PC13 (idle HIGH via external pull-up, LOW when pressed) */
    gpio_init_input(GPIOC, 13);

    /* USART2 TX/RX pins: PA2 = TX, PA3 = RX, both use AF7 */
    gpio_init_af(GPIOA, 2, 7);
    gpio_init_af(GPIOA, 3, 7);

    /* PA0 (Arduino A0) as an analog input for ADC1 channel 0 */
    gpio_init_analog(GPIOA, 0);

    uart_init(USART2, 115200, 16000000);
    console_init();
    systick_init(16000 - 1);  /* 1ms tick @ 16MHz HSI */
    kmalloc_init();
    adc_init();

    /* Initialize EXTI for Blue Button (NVIC Demo) */
    button_exti_init();
    irq_enable_global(); /* Ensure interrupts are unmasked */

    console_puts("\r\n--- emOS Core Interrupts Booted ---\r\n");
    console_puts("Commands:  led on | led off | led toggle | button | adc | adc vref | gpio test | uart test | stats | fault | mem | memtest\r\n");
    console_puts("Press the Blue Button at any time to trigger an interrupt!\r\n");

    while (1) {
        console_puts("\r\n> ");
        console_readline(line_buf, sizeof(line_buf));
        console_puts("\r\n");

        if (str_equals(line_buf, "led on")) {
            gpio_write(GPIOA, 5, GPIO_PIN_HIGH);
            console_puts("LED turned ON\r\n");
        }

        else if (str_equals(line_buf, "led off")) {
            gpio_write(GPIOA, 5, GPIO_PIN_LOW);
            console_puts("LED turned OFF\r\n");
        }

        else if (str_equals(line_buf, "led toggle")) {
            gpio_toggle(GPIOA, 5);
            console_puts("LED toggled\r\n");
        } else if (str_equals(line_buf, "button")) {
            if (gpio_read(GPIOC, 13) == GPIO_PIN_LOW) {
                console_puts("Button is PRESSED\r\n");
            } else {
                console_puts("Button is NOT pressed\r\n");
            }
        } else if (str_equals(line_buf, "mem")) {
            cmd_mem();
        } else if (str_equals(line_buf, "memtest")) {
            cmd_memtest();
        } else if (str_equals(line_buf, "adc")) {
            cmd_adc();
        } else if (str_equals(line_buf, "adc vref")) {
            cmd_adc_vref();
        } else if (str_equals(line_buf, "gpio test")) {
            selftest_gpio();
        } else if (str_equals(line_buf, "uart test")) {
            selftest_uart();
        } else if (line_buf[0] == '\0') {
            /* Empty line (just pressed Enter) -- do nothing, reprint prompt. */
        } 
        else if (str_equals(line_buf, "stats")) {
            console_puts("Button interrupt trigger count: ");
            console_put_uint(button_press_count);
            console_puts("\r\n");
        }

        else if (str_equals(line_buf, "fault")) {
            console_puts("Simulating HardFault via undefined instruction.....\r\n");
            __asm volatile("udf #0");   /* UsageFault -> escalates to HardFault */
        }

        else {
            console_puts("Unknown command. Try: led on | led off | led toggle | button | adc | adc vref | gpio test | uart test | stats | fault | mem | memtest\r\n");
        }
    }

    return 0;
}