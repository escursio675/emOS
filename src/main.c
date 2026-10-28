#include <stdint.h>
#include <stddef.h>
#include "systick.h"
#include "gpio.h"
#include "uart.h"
#include "../inc/memory.h"
#include "interrupts.h"

/* Small local string-equality helper -- avoids pulling in <string.h>
 * for a single comparison, matching this project's minimal-dependency
 * bare-metal style. */
static int str_equals(const char *a, const char *b)
{
    while (*a != '\0' && *b != '\0') {
        if (*a != *b) {
            return 0;
        }
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

static void say(const char *s)
{
    uart_puts(USART2, s);
}

static void say_uint(uint32_t v)
{
    char buf[11];            /* 10 digits + NUL */
    uint32_t i = 10U;

    buf[i] = '\0';
    do {
        i--;
        buf[i] = (char)('0' + (v % 10U));
        v /= 10U;
    } while (v != 0U);

    say(&buf[i]);
}

static void say_hex32(uint32_t v)
{
    static const char digits[] = "0123456789abcdef";
    char buf[11];
    int i;

    buf[0] = '0';
    buf[1] = 'x';
    for (i = 0; i < 8; i++) {
        buf[2 + i] = digits[(v >> (28 - 4 * i)) & 0xFU];
    }
    buf[10] = '\0';
    say(buf);
}

/* mem: allocator statistics */
static void cmd_mem(void)
{
    heap_stats_t s;

    kmalloc_stats(&s);

    say("Heap:\r\n");
    say("  Total:        "); say_uint((uint32_t)s.total_bytes);  say("\r\n");
    say("  Used:         "); say_uint((uint32_t)s.used_bytes);   say("\r\n");
    say("  Free:         "); say_uint((uint32_t)s.free_bytes);   say("\r\n");
    say("  Largest free: "); say_uint((uint32_t)s.largest_free); say("\r\n");
    say("  Blocks:       "); say_uint((uint32_t)s.block_count);  say("\r\n");
}

/* memtest: exercises kmalloc/kfree on the board. Temporary test command. */
static void cmd_memtest(void)
{
    void *a = kmalloc(100);
    void *b = kmalloc(200);
    void *c;
    void *big;

    say("a = kmalloc(100): "); say_hex32((uint32_t)(uintptr_t)a); say("\r\n");
    say("b = kmalloc(200): "); say_hex32((uint32_t)(uintptr_t)b); say("\r\n");
    cmd_mem();

    kfree(a);
    c = kmalloc(50);
    say("kfree(a); c = kmalloc(50): "); say_hex32((uint32_t)(uintptr_t)c);
    say(c == a ? " (reused a's block)\r\n" : " (different block)\r\n");

    big = kmalloc(KHEAP_SIZE);
    say("kmalloc(KHEAP_SIZE): ");
    say(big == NULL ? "NULL (out of memory, as expected)\r\n" : "UNEXPECTED: succeeded\r\n");

    kfree(b);
    kfree(c);
    kfree(big);              /* NULL is ignored */
    say("freed everything\r\n");
    cmd_mem();
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

    uart_init(USART2, 115200, 16000000);
    systick_init(16000 - 1);  /* 1ms tick @ 16MHz HSI */
    kmalloc_init();

    /* Initialize EXTI for Blue Button (NVIC Demo) */
    button_exti_init();
    irq_enable_global(); /* Ensure interrupts are unmasked */

    uart_puts(USART2, "\r\n--- emOS Core Interrupts Booted ---\r\n");
    uart_puts(USART2, "Commands:  led on | led off | led toggle | stats | fault | mem | memtest\r\n");
    uart_puts(USART2, "Press the Blue Button at any time to trigger an interrupt!\r\n");

    while (1) {
        uart_puts(USART2, "\r\n> ");
        uart_read_line(USART2, line_buf, sizeof(line_buf));
        uart_puts(USART2, "\r\n");

        if (str_equals(line_buf, "led on")) {
            gpio_write(GPIOA, 5, GPIO_PIN_HIGH);
            uart_puts(USART2, "LED turned ON\r\n");
        }

        else if (str_equals(line_buf, "led off")) {
            gpio_write(GPIOA, 5, GPIO_PIN_LOW);
            uart_puts(USART2, "LED turned OFF\r\n");
        }

        else if (str_equals(line_buf, "led toggle")) {
            gpio_toggle(GPIOA, 5);
            uart_puts(USART2, "LED toggled\r\n");
        } else if (str_equals(line_buf, "button")) {
            if (gpio_read(GPIOC, 13) == GPIO_PIN_LOW) {
                uart_puts(USART2, "Button is PRESSED\r\n");
            } else {
                uart_puts(USART2, "Button is NOT pressed\r\n");
            }
        } else if (str_equals(line_buf, "mem")) {
            cmd_mem();
        } else if (str_equals(line_buf, "memtest")) {
            cmd_memtest();
        } else if (line_buf[0] == '\0') {
            /* Empty line (just pressed Enter) -- do nothing, reprint prompt. */
        } 
        else if (str_equals(line_buf, "stats")) {

            /* basic integer printing to show the interrupt variable changing */

            uart_puts(USART2, "Button interrupt trigger count: ");
            char count_str[10];
            int i = 0;
            uint32_t temp = button_press_count;
            if (temp == 0) count_str[i++] = '0';
            while (temp > 0 && i < 9) { count_str[i++] = (temp % 10) + '0'; temp /= 10; }
            while (i > 0) uart_putc(USART2, count_str[--i]);
            uart_puts(USART2, "\r\n");
        }

        else if (str_equals(line_buf, "fault")) {
            uart_puts(USART2, "Simulating HardFault via undefined instruction.....\r\n");
            __asm volatile("udf #0");   /* UsageFault -> escalates to HardFault */
        }

        else if (line_buf[0] == '\0') {
            /* Empty line (just pressed Enter) -- do nothing, reprint prompt. */
        }

        else {
            uart_puts(USART2, "Unknown command. Try: led on | led off | led toggle | stats | fault | mem | memtest\r\n");
        }
    }

    return 0;
}